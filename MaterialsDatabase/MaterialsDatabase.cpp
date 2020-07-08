/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MaterialsDatabase.h"
#include "StringFunctions.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <fstream>
#include <sstream>

using namespace StringFunctions;

CMaterialsDatabase::CMaterialsDatabase()
{
	m_sFileName = MDBDescriptors::DEFAULT_MDB_FILE_NAME;

	activeConstProperties = MDBDescriptors::defaultConstProperties;
	activeTPDepProperties = MDBDescriptors::defaultTPDProperties;
	activeInterProperties = MDBDescriptors::defaultInteractionProperties;
}

MDBDescriptors::constDescr CMaterialsDatabase::ActiveConstProperties() const
{
	return activeConstProperties;
}

MDBDescriptors::tpdepDescr CMaterialsDatabase::ActiveTPDepProperties() const
{
	return activeTPDepProperties;
}

MDBDescriptors::interDescr CMaterialsDatabase::ActiveInterProperties() const
{
	return activeInterProperties;
}

void CMaterialsDatabase::AddProperty(const MDBDescriptors::SPropertyDescriptor& _descriptor)
{
	// get value to check _key
	unsigned firstKey{ MDBDescriptors::FIRST_CONST_USER_PROP };
	switch (_descriptor.type)
	{
	case MDBDescriptors::EPropertyType::CONSTANT:		firstKey = MDBDescriptors::FIRST_CONST_USER_PROP;	break;
	case MDBDescriptors::EPropertyType::TP_DEPENDENT:	firstKey = MDBDescriptors::FIRST_TPDEP_USER_PROP;	break;
	case MDBDescriptors::EPropertyType::INTERACTION:	firstKey = MDBDescriptors::FIRST_INTER_USER_PROP;	break;
	}

	// check _key and add a property
	MDBDescriptors::SCompoundPropertyDescriptor* descr{ nullptr };
	if (firstKey <= _descriptor.key && _descriptor.key < firstKey + MDBDescriptors::MAX_USER_DEFINED_PROP_NUMBER)
		switch (_descriptor.type)
		{
		case MDBDescriptors::EPropertyType::CONSTANT:		descr = &activeConstProperties[static_cast<ECompoundConstProperties>(_descriptor.key)];	break;
		case MDBDescriptors::EPropertyType::TP_DEPENDENT:	descr = &activeTPDepProperties[static_cast<ECompoundTPProperties>(_descriptor.key)];	break;
		case MDBDescriptors::EPropertyType::INTERACTION:	descr = &activeInterProperties[static_cast<EInteractionProperties>(_descriptor.key)];	break;
		}

	if (!descr) return;

	// set values
	descr->name = _descriptor.name;
	descr->units = _descriptor.units;
	descr->description = _descriptor.description;
	if (_descriptor.type == MDBDescriptors::EPropertyType::CONSTANT)
		dynamic_cast<MDBDescriptors::SCompoundConstPropertyDescriptor*>(descr)->defaultValue = _descriptor.value;
	else
	{
		dynamic_cast<MDBDescriptors::SCompoundTPDPropertyDescriptor*>(descr)->defuaultType = ECorrelationTypes::CONSTANT;
		dynamic_cast<MDBDescriptors::SCompoundTPDPropertyDescriptor*>(descr)->defaultParameters = { _descriptor.value };
	}

	// add to compounds and interactions
	switch (_descriptor.type)
	{
	case MDBDescriptors::EPropertyType::CONSTANT:
		for (auto& c : m_vCompounds)
			c.AddConstProperty(static_cast<ECompoundConstProperties>(_descriptor.key), _descriptor.name, _descriptor.units, _descriptor.value);
		break;
	case MDBDescriptors::EPropertyType::TP_DEPENDENT:
		for (auto& c : m_vCompounds)
			c.AddTPDepProperty(static_cast<ECompoundTPProperties>(_descriptor.key), _descriptor.name, _descriptor.units, _descriptor.value);
		break;
	case MDBDescriptors::EPropertyType::INTERACTION:
		for (auto& i : m_vInteractions)
			i.AddProperty(static_cast<EInteractionProperties>(_descriptor.key), _descriptor.name, _descriptor.units, _descriptor.value);
		break;
	}
}

void CMaterialsDatabase::RemoveProperty(const MDBDescriptors::EPropertyType& _type, unsigned _key)
{
	switch (_type)
	{
	case MDBDescriptors::EPropertyType::CONSTANT:
		activeConstProperties.erase(static_cast<ECompoundConstProperties>(_key));
		for (auto& c : m_vCompounds)
			c.RemoveConstProperty(static_cast<ECompoundConstProperties>(_key));
		break;
	case MDBDescriptors::EPropertyType::TP_DEPENDENT:
		activeTPDepProperties.erase(static_cast<ECompoundTPProperties>(_key));
		for (auto& c : m_vCompounds)
			c.RemoveTPDepProperty(static_cast<ECompoundTPProperties>(_key));
		break;
	case MDBDescriptors::EPropertyType::INTERACTION:
		activeInterProperties.erase(static_cast<EInteractionProperties>(_key));
		for (auto& i : m_vInteractions)
			i.RemoveProperty(static_cast<EInteractionProperties>(_key));
		break;
	}
}

bool CMaterialsDatabase::IsPropertyDefined(unsigned _key) const
{
	if (MapContainsKey(activeConstProperties, static_cast<ECompoundConstProperties>(_key)))
		return true;
	if (MapContainsKey(activeTPDepProperties, static_cast<ECompoundTPProperties>(_key)))
		return true;
	if (MapContainsKey(activeInterProperties, static_cast<EInteractionProperties>(_key)))
		return true;
	return false;
}

std::wstring CMaterialsDatabase::GetFileName() const
{
	return m_sFileName;
}

void CMaterialsDatabase::CreateNewDatabase()
{
	m_sFileName.clear();
	m_vCompounds.clear();
	m_vInteractions.clear();

	activeConstProperties = MDBDescriptors::defaultConstProperties;
	activeTPDepProperties = MDBDescriptors::defaultTPDProperties;
	activeInterProperties = MDBDescriptors::defaultInteractionProperties;
}

bool CMaterialsDatabase::SaveToFile(const std::wstring& _fileName /*= ""*/)
{
	// Writes information about user-defined types into file
	const auto WritePropInfo = [](std::ofstream& _outFile, MDBDescriptors::EPropertyType _type, unsigned _key, const MDBDescriptors::SCompoundPropertyDescriptor& _descr)
	{
		const std::string constBeg = std::to_string(MDBDescriptors::FIRST_CONST_USER_PROP);
		const std::string constEnd = std::to_string(MDBDescriptors::FIRST_CONST_USER_PROP + MDBDescriptors::MAX_USER_DEFINED_PROP_NUMBER);
		const std::string tpDepBeg = std::to_string(MDBDescriptors::FIRST_TPDEP_USER_PROP);
		const std::string tpDepEnd = std::to_string(MDBDescriptors::FIRST_TPDEP_USER_PROP + MDBDescriptors::MAX_USER_DEFINED_PROP_NUMBER);
		const std::string interBeg = std::to_string(MDBDescriptors::FIRST_INTER_USER_PROP);
		const std::string interEnd = std::to_string(MDBDescriptors::FIRST_INTER_USER_PROP + MDBDescriptors::MAX_USER_DEFINED_PROP_NUMBER);
		const std::string ranges = "[" + constBeg + ".." + constEnd + "] / [" + tpDepBeg + ".." + tpDepEnd + "] / [" + interBeg + ".." + interEnd + "]";
		_outFile << E2I(ETXTFileKeys::PROPERTY_TYPE) << " " << E2I(_type) << Comment("Type: (0 - Constant) / (1 - T/P-dependent) / (2 - Interaction)") << std::endl;
		_outFile << E2I(ETXTFileKeys::PROPERTY_KEY) << " " << _key << Comment("Unique key " + ranges) << std::endl;
		_outFile << E2I(ETXTFileKeys::PROPERTY_NAME) << " " << _descr.name << Comment("Property name") << std::endl;
		_outFile << E2I(ETXTFileKeys::PROPERTY_UNITS) << " " << WString2String(_descr.units) << Comment("Measurement units") << std::endl;
		if (_type == MDBDescriptors::EPropertyType::CONSTANT)
			_outFile << E2I(ETXTFileKeys::PROPERTY_INIT_VALUE) << " " << dynamic_cast<const MDBDescriptors::SCompoundConstPropertyDescriptor&>(_descr).defaultValue;
		else
		{
			_outFile << E2I(ETXTFileKeys::PROPERTY_INIT_VALUE) << " " << E2I(dynamic_cast<const MDBDescriptors::SCompoundTPDPropertyDescriptor&>(_descr).defuaultType);
			for (double v : dynamic_cast<const MDBDescriptors::SCompoundTPDPropertyDescriptor&>(_descr).defaultParameters)
				_outFile << " " << v;
		}
		_outFile << Comment("Initial value") << std::endl;
		_outFile << E2I(ETXTFileKeys::PROPERTY_DESCRIPTION) << " " << ReplaceAll(_descr.description, "\n", MDBDescriptors::NEW_LINE_REPLACER) << Comment("Description") << std::endl << std::endl;
	};

	const std::wstring fileName = _fileName.empty() ? m_sFileName : _fileName;
	std::ofstream outFile(UnicodePath(fileName));
	if (outFile.fail()) return false;

	// write signature and version number
	outFile << MDBDescriptors::SIGNATURE_STRING << " " << MDBDescriptors::VERSION << std::endl << std::endl;

	// save additional const properties
	for (const auto& p : activeConstProperties) // for all current properties
		if (!MapContainsKey(MDBDescriptors::defaultConstProperties, p.first)) // new non-default property found
			WritePropInfo(outFile, MDBDescriptors::EPropertyType::CONSTANT, p.first, p.second);
	// save additional dependent properties
	for (const auto& p : activeTPDepProperties) // for all current properties
		if (!MapContainsKey(MDBDescriptors::defaultTPDProperties, p.first)) // new non-default property found
			WritePropInfo(outFile, MDBDescriptors::EPropertyType::TP_DEPENDENT, p.first, p.second);
	// save additional interaction properties
	for (const auto& p : activeInterProperties) // for all current properties
		if (!MapContainsKey(MDBDescriptors::defaultInteractionProperties, p.first)) // new non-default property found
			WritePropInfo(outFile, MDBDescriptors::EPropertyType::INTERACTION, p.first, p.second);

	// save compounds
	for (const auto& compound : m_vCompounds)
	{
		outFile << E2I(ETXTFileKeys::COMPOUND_KEY)  << " " << compound.GetKey()  << Comment("Compound's unique key") << std::endl;
		outFile << E2I(ETXTFileKeys::COMPOUND_NAME) << " " << compound.GetName() << Comment("Compound's name")       << std::endl;
		if(!compound.GetDescription().empty())
			outFile << E2I(ETXTFileKeys::COMPOUND_DESCRIPTION) << " " << compound.GetDescription() << Comment("Compound's description") << std::endl;

		// const properties
		for (const auto& prop : compound.GetConstProperties())
		{
			if (!prop.IsDefaultValue()) // do not store default values
			{
				const auto& propDescr = activeConstProperties[static_cast<ECompoundConstProperties>(prop.GetType())];
				outFile << E2I(ETXTFileKeys::COMPOUND_CONST_PROPERTY) << " " << prop.GetType() << " " << prop.GetValue()
					<< Comment(propDescr.name + " [" + WString2String(propDescr.units) + "]") << std::endl;
			}
			if (!prop.GetDescription().empty()) // save user provided description of const property
				outFile << E2I(ETXTFileKeys::CONST_PROPERTY_DESCRIPTION) << " " << prop.GetType() << " " << prop.GetDescription()
				<< Comment("User provided description for const property") << std::endl;
		}

		// dependent properties
		for (const auto& prop : compound.GetTPProperties())
		{
			const auto propDescr = activeTPDepProperties[static_cast<ECompoundTPProperties>(prop.GetType())];
			for (size_t i = 0; i < prop.CorrelationsNumber(); ++i)
			{
				const CCorrelation& corr = *prop.GetCorrelation(i);
				const auto corrDescr = MDBDescriptors::correlations[corr.GetType()];
				if (!prop.IsDefaultValue()) // do not store default values
					outFile << E2I(ETXTFileKeys::COMPOUND_TPD_PROPERTY) << " " << prop.GetType() << " " << corr
						<< Comment(propDescr.name + " [" + WString2String(propDescr.units) + "]; " + corrDescr.name) << std::endl;
				if (!corr.GetDescription().empty()) // save user provided description of correlation
					outFile << E2I(ETXTFileKeys::CORRELATION_DESCRIPTION) << " " << prop.GetType() << " " << i << " " << corr.GetDescription()
						<< Comment("User provided description for correlation") << std::endl;
			}
			if (!prop.GetDescription().empty()) // save user provided description of TP property
				outFile << E2I(ETXTFileKeys::TPD_PROPERTY_DESCRIPTION) << " " << prop.GetType() << " " << prop.GetDescription()
					<< Comment("User provided description for dependent property") << std::endl;
		}
		outFile << std::endl;
	}

	// save interactions
	for (const auto& interaction : m_vInteractions)
	{
		// check if there is something to save
		size_t toSave = 0;
		for (const auto& prop : interaction.GetProperties())
			if (!prop.IsDefaultValue())
				++toSave;
		if (toSave == 0) continue;

		outFile << E2I(ETXTFileKeys::INTERACTION_KEYS) << " " << interaction.GetKey1() << " " << interaction.GetKey2() << Comment("Unique keys of interacting compounds") << std::endl;
		for (const auto& prop : interaction.GetProperties())
		{
			const auto interDescr = activeInterProperties[static_cast<EInteractionProperties>(prop.GetType())];
			for (size_t i = 0; i < prop.CorrelationsNumber(); ++i)
			{
				const CCorrelation& corr = *prop.GetCorrelation(i);
				if (!prop.IsDefaultValue()) // do not store default values
				{
					const auto corrDescr = MDBDescriptors::correlations[corr.GetType()];
					outFile << E2I(ETXTFileKeys::INTERACTION_PROPERTY) << " " << prop.GetType() << " " << corr
						<< Comment(interDescr.name + " [" + WString2String(interDescr.units) + "]; " + corrDescr.name) << std::endl;
				}
				if (!corr.GetDescription().empty()) // save user provided description of correlation
					outFile << E2I(ETXTFileKeys::INTERACTION_CORRELATION_DESCRIPTION) << " " << prop.GetType() << " " << i << corr.GetDescription()
					<< Comment("User provided description for interaction's correlation") << std::endl;
			}
			if (!prop.GetDescription().empty()) // save user provided description of TP property
				outFile << E2I(ETXTFileKeys::INTERACTION_TPD_PROPERTY_DESCRIPTION) << " " << prop.GetType() << " " << prop.GetDescription()
					<< Comment("User provided description for interaction's dependent property") << std::endl;
		}
		outFile << std::endl;
	}
	m_sFileName = fileName;
	return true;
}

bool CMaterialsDatabase::LoadFromFile(const std::wstring& _fileName /*= ""*/)
{
	CreateNewDatabase();
	const std::wstring fileName = _fileName.empty() ? MDBDescriptors::DEFAULT_MDB_FILE_NAME : _fileName;

	std::ifstream inFile(UnicodePath(fileName));
	if (inFile.fail()) return false;

	// read signature and version number
	const std::string signature = GetValueFromStream<std::string>(&inFile);
	const auto version = GetValueFromStream<unsigned>(&inFile);
	bool res;
	if (signature != MDBDescriptors::SIGNATURE_STRING) // old or wrong file
		res = LoadFromFileV0(inFile);			// try to load old format v0
	else if (version == 1 || version == 2)
		res = LoadFromFileV2(inFile);			// try to load old format v1/v2
	else
		res = LoadFromFileV3(inFile);			// try to load old format v3

	if (res)
		m_sFileName = fileName;
	return res;
}

bool CMaterialsDatabase::LoadFromFileV0(std::ifstream& _file)
{
	_file.seekg(0, std::ifstream::beg); // go to the beginning of the file

	bool ret = false; // return flag
	CCompound* pCurrCompound = nullptr;
	std::string sLine;
	while (std::getline(_file, sLine))
	{
		if (sLine.empty()) continue;
		std::stringstream ss(sLine);
		const auto mainKey = static_cast<ETXTFileKeysOld>(GetValueFromStream<unsigned>(&ss));
		const auto subKey = static_cast<ETXTFileKeysOld>(GetValueFromStream<unsigned>(&ss));

		if (mainKey == ETXTFileKeysOld::MDB_PROPERTY && subKey == ETXTFileKeysOld::MDB_PROPERTY_SOA)
			ret = true;

		if (mainKey == ETXTFileKeysOld::MDB_COMPOUND)
			switch (subKey)
			{
			case ETXTFileKeysOld::COMPOUND_KEY:
				pCurrCompound = AddCompound(TrimFromSymbols(GetValueFromStream<std::string>(&ss), StrConst::COMMENT_SYMBOL));
				break;
			case ETXTFileKeysOld::COMPOUND_NAME:
				if (!pCurrCompound) continue;
				pCurrCompound->SetName(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
				break;
			case ETXTFileKeysOld::COMPOUND_PROPERTY:
			{
				if (!pCurrCompound) continue;
				auto propKey = GetValueFromStream<unsigned>(&ss);
				if (CConstProperty* pConstProp = pCurrCompound->GetConstProperty(static_cast<ECompoundConstProperties>(propKey)))
					pConstProp->SetValue(GetValueFromStream<double>(&ss));
				else if (CTPDProperty* pTPDProp = pCurrCompound->GetTPProperty(static_cast<ECompoundTPProperties>(propKey)))
				{
					std::stringstream valuesStream(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
					std::vector<double> values;
					while (!valuesStream.eof())
					{
						double dTemp;
						valuesStream >> dTemp;
						values.push_back(dTemp);
					}
					pTPDProp->RemoveAllCorrelations();
					if (values.size() == 1)
						pTPDProp->AddCorrelation(ECorrelationTypes::CONSTANT, values);
					else if (values.size() == 2)
						pTPDProp->AddCorrelation(ECorrelationTypes::CONSTANT, { values[1] });
					else
					{
						if (values.size() % 2 != 0)
							values.resize(values.size() + 1, 0);
						pTPDProp->AddCorrelation(ECorrelationTypes::LIST_OF_T_VALUES, values);
						std::vector<double> vParams = pTPDProp->GetCorrelation(0)->GetParameters();
						pTPDProp->GetCorrelation(0)->SetTInterval(SInterval{ vParams[0], vParams[vParams.size() - 2] });
					}
				}
				break;
			}
			default: break;
			}
	}
	return ret;
}

bool CMaterialsDatabase::LoadFromFileV2(std::ifstream& _file)
{
	CCompound* pCurrCompound = nullptr;
	CInteraction* pCurrInteraction = nullptr;
	std::string sLine;
	while (std::getline(_file, sLine))
	{
		if (sLine.empty()) continue;
		std::stringstream ss(sLine);

		switch (static_cast<ETXTFileKeys>(GetValueFromStream<unsigned>(&ss)))
		{
		case ETXTFileKeys::COMPOUND_KEY:
			pCurrCompound = AddCompound(TrimFromSymbols(GetValueFromStream<std::string>(&ss), StrConst::COMMENT_SYMBOL));
			for (size_t i = 0; i < pCurrCompound->TPPropertiesNumber(); ++i) // remove all default correlations from temperature/pressure-dependent properties
				pCurrCompound->GetTPPropertyByIndex(i)->RemoveAllCorrelations();
			break;
		case ETXTFileKeys::COMPOUND_NAME:
			if (!pCurrCompound) continue;
			pCurrCompound->SetName(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::COMPOUND_DESCRIPTION:
			if (!pCurrCompound) continue;
			pCurrCompound->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::COMPOUND_CONST_PROPERTY:
			if (!pCurrCompound) continue;
			if (CConstProperty* pProp = pCurrCompound->GetConstProperty(static_cast<ECompoundConstProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetValue(GetValueFromStream<double>(&ss));
			break;
		case ETXTFileKeys::COMPOUND_TPD_PROPERTY:
			if (!pCurrCompound) continue;
			if (CTPDProperty* pProp = pCurrCompound->GetTPProperty(static_cast<ECompoundTPProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->AddCorrelation(GetValueFromStream<CCorrelation>(&ss));
			break;
		case ETXTFileKeys::INTERACTION_KEYS:
			pCurrInteraction = AddInteraction(GetValueFromStream<std::string>(&ss), GetValueFromStream<std::string>(&ss));
			for (size_t i = 0; i < pCurrInteraction->PropertiesNumber(); ++i) // remove all default correlations from temperature/pressure-dependent properties
				pCurrInteraction->GetPropertyByIndex(i)->RemoveAllCorrelations();
			break;
		case ETXTFileKeys::INTERACTION_PROPERTY:
			if (!pCurrInteraction) continue;
			if (CTPDProperty* pProp = pCurrInteraction->GetProperty(static_cast<EInteractionProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->AddCorrelation(GetValueFromStream<CCorrelation>(&ss));
			break;
		case ETXTFileKeys::CONST_PROPERTY_DESCRIPTION:
			if (!pCurrCompound) continue;
			if (CConstProperty* pProp = pCurrCompound->GetConstProperty(static_cast<ECompoundConstProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::TPD_PROPERTY_DESCRIPTION:
			if (!pCurrCompound) continue;
			if (CTPDProperty* pProp = pCurrCompound->GetTPProperty(static_cast<ECompoundTPProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::CORRELATION_DESCRIPTION:
			if (!pCurrCompound) continue;
			if (CTPDProperty* pProp = pCurrCompound->GetTPProperty(static_cast<ECompoundTPProperties>(GetValueFromStream<unsigned>(&ss))))
				if (CCorrelation* pCorr = pProp->GetCorrelation(GetValueFromStream<unsigned>(&ss)))
					pCorr->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::INTERACTION_TPD_PROPERTY_DESCRIPTION:
			if (!pCurrInteraction) continue;
			if (CTPDProperty* pProp = pCurrInteraction->GetProperty(static_cast<EInteractionProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::INTERACTION_CORRELATION_DESCRIPTION:
			if (!pCurrInteraction) continue;
			if (CTPDProperty* pProp = pCurrInteraction->GetProperty(static_cast<EInteractionProperties>(GetValueFromStream<unsigned>(&ss))))
				if (CCorrelation* pCorr = pProp->GetCorrelation(GetValueFromStream<unsigned>(&ss)))
					pCorr->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		default: break;
		}
	}
	return true;
}

bool CMaterialsDatabase::LoadFromFileV3(std::ifstream& _file)
{
	MDBDescriptors::SCompoundPropertyDescriptor* currPropDescr{};
	CCompound* pCurrCompound{};
	CInteraction* pCurrInteraction{};
	MDBDescriptors::EPropertyType currPropType{};
	ECompoundTPProperties lastTPProp{ TP_PROP_NO_PROERTY };    // used to track whether to delete the default correlation
	EInteractionProperties lastIntProp{ INT_PROP_NO_PROERTY }; // used to track whether to delete the default correlation
	std::string line;
	while (std::getline(_file, line))
	{
		if (line.empty()) continue;
		std::stringstream ss(line);

		switch (static_cast<ETXTFileKeys>(GetValueFromStream<unsigned>(&ss)))
		{
		case ETXTFileKeys::PROPERTY_TYPE:
			currPropType = static_cast<MDBDescriptors::EPropertyType>(GetValueFromStream<unsigned>(&ss));
			break;
		case ETXTFileKeys::PROPERTY_KEY:
			switch (currPropType)
			{
			case MDBDescriptors::EPropertyType::CONSTANT:	  currPropDescr = &activeConstProperties[static_cast<ECompoundConstProperties>(GetValueFromStream<unsigned>(&ss))]; break;
			case MDBDescriptors::EPropertyType::TP_DEPENDENT: currPropDescr = &activeTPDepProperties[static_cast<ECompoundTPProperties>(GetValueFromStream<unsigned>(&ss))];	break;
			case MDBDescriptors::EPropertyType::INTERACTION:  currPropDescr = &activeInterProperties[static_cast<EInteractionProperties>(GetValueFromStream<unsigned>(&ss))];	break;
			default:;
			}
			break;
		case ETXTFileKeys::PROPERTY_NAME:
			currPropDescr->name = TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL);
			break;
		case ETXTFileKeys::PROPERTY_UNITS:
			currPropDescr->units = String2WString(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::PROPERTY_INIT_VALUE:
			if (currPropType == MDBDescriptors::EPropertyType::CONSTANT)
				dynamic_cast<MDBDescriptors::SCompoundConstPropertyDescriptor*>(currPropDescr)->defaultValue = GetValueFromStream<double>(&ss);
			else
			{
				const auto defaultType = static_cast<ECorrelationTypes>(GetValueFromStream<unsigned>(&ss));
				std::vector<double> defaultParameters;
				for (size_t i = 0; i < MDBDescriptors::correlations[defaultType].parametersNumber; ++i)
					defaultParameters.push_back(GetValueFromStream<double>(&ss));
				dynamic_cast<MDBDescriptors::SCompoundTPDPropertyDescriptor*>(currPropDescr)->defuaultType = defaultType;
				dynamic_cast<MDBDescriptors::SCompoundTPDPropertyDescriptor*>(currPropDescr)->defaultParameters = defaultParameters;
				break;
			}
			break;
		case ETXTFileKeys::PROPERTY_DESCRIPTION:
			currPropDescr->description = ReplaceAll(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL), MDBDescriptors::NEW_LINE_REPLACER, "\n");
			break;
		case ETXTFileKeys::COMPOUND_KEY:
			pCurrCompound = AddCompound(TrimFromSymbols(GetValueFromStream<std::string>(&ss), StrConst::COMMENT_SYMBOL));
			lastTPProp = TP_PROP_NO_PROERTY;
			break;
		case ETXTFileKeys::COMPOUND_NAME:
			if (!pCurrCompound) continue;
			pCurrCompound->SetName(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::COMPOUND_DESCRIPTION:
			if (!pCurrCompound) continue;
			pCurrCompound->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::COMPOUND_CONST_PROPERTY:
			if (!pCurrCompound) continue;
			if (CConstProperty* pProp = pCurrCompound->GetConstProperty(static_cast<ECompoundConstProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetValue(GetValueFromStream<double>(&ss));
			break;
		case ETXTFileKeys::COMPOUND_TPD_PROPERTY:
		{
			if (!pCurrCompound) continue;
			const auto TPProp = static_cast<ECompoundTPProperties>(GetValueFromStream<unsigned>(&ss));
			if (CTPDProperty* pProp = pCurrCompound->GetTPProperty(TPProp))
			{
				if (TPProp != lastTPProp) // first met this property for this compound - remove default correlation
					pProp->RemoveCorrelation(0);
				pProp->AddCorrelation(GetValueFromStream<CCorrelation>(&ss));
				lastTPProp = TPProp;
			}
			break;
		}
		case ETXTFileKeys::INTERACTION_KEYS:
			pCurrInteraction = AddInteraction(GetValueFromStream<std::string>(&ss), GetValueFromStream<std::string>(&ss));
			lastIntProp = INT_PROP_NO_PROERTY;
			break;
		case ETXTFileKeys::INTERACTION_PROPERTY:
		{
			if (!pCurrInteraction) continue;
			const auto IntProp = static_cast<EInteractionProperties>(GetValueFromStream<unsigned>(&ss));
			if (CTPDProperty* pProp = pCurrInteraction->GetProperty(IntProp))
			{
				if (IntProp != lastIntProp) // first met this property for this compound - remove default correlation
					pProp->RemoveCorrelation(0);
				pProp->AddCorrelation(GetValueFromStream<CCorrelation>(&ss));
				lastIntProp = IntProp;
			}
			break;
		}
		case ETXTFileKeys::CONST_PROPERTY_DESCRIPTION:
			if (!pCurrCompound) continue;
			if (CConstProperty* pProp = pCurrCompound->GetConstProperty(static_cast<ECompoundConstProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::TPD_PROPERTY_DESCRIPTION:
			if (!pCurrCompound) continue;
			if (CTPDProperty* pProp = pCurrCompound->GetTPProperty(static_cast<ECompoundTPProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::CORRELATION_DESCRIPTION:
			if (!pCurrCompound) continue;
			if (CTPDProperty* pProp = pCurrCompound->GetTPProperty(static_cast<ECompoundTPProperties>(GetValueFromStream<unsigned>(&ss))))
				if (CCorrelation* pCorr = pProp->GetCorrelation(GetValueFromStream<unsigned>(&ss)))
					pCorr->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::INTERACTION_TPD_PROPERTY_DESCRIPTION:
			if (!pCurrInteraction) continue;
			if (CTPDProperty* pProp = pCurrInteraction->GetProperty(static_cast<EInteractionProperties>(GetValueFromStream<unsigned>(&ss))))
				pProp->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case ETXTFileKeys::INTERACTION_CORRELATION_DESCRIPTION:
			if (!pCurrInteraction) continue;
			if (CTPDProperty* pProp = pCurrInteraction->GetProperty(static_cast<EInteractionProperties>(GetValueFromStream<unsigned>(&ss))))
				if (CCorrelation* pCorr = pProp->GetCorrelation(GetValueFromStream<unsigned>(&ss)))
					pCorr->SetDescription(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		}
	}
	return true;
}

size_t CMaterialsDatabase::CompoundsNumber() const
{
	return m_vCompounds.size();
}

CCompound* CMaterialsDatabase::AddCompound(const std::string& _sCompoundUniqueKey /*= ""*/)
{
	return AddCompound(CCompound{ activeConstProperties, activeTPDepProperties, _sCompoundUniqueKey });
}

CCompound* CMaterialsDatabase::AddCompound(const CCompound& _compound)
{
	// generate unique key
	const std::string sKey = GenerateUniqueString(_compound.GetKey(), GetCompoundsKeys());
	// add new compound
	m_vCompounds.emplace_back(_compound);
	// set key
	m_vCompounds.back().SetKey(sKey);
	// add corresponding interactions
	ConformInteractionsAdd(sKey);
	// return pointer to added compound
	return &m_vCompounds.back();
}

void CMaterialsDatabase::RemoveCompound(size_t _iCompound)
{
	if (_iCompound >= m_vCompounds.size()) return;
	ConformInteractionsRemove(m_vCompounds[_iCompound].GetKey());
	m_vCompounds.erase(m_vCompounds.begin() + _iCompound);
}

void CMaterialsDatabase::RemoveCompound(const std::string& _sCompoundUniqueKey)
{
	RemoveCompound(GetCompoundIndex(_sCompoundUniqueKey));
}

void CMaterialsDatabase::ShiftCompoundUp(size_t _iCompound)
{
	if (_iCompound < m_vCompounds.size() && _iCompound != 0)
		std::iter_swap(m_vCompounds.begin() + _iCompound, m_vCompounds.begin() + _iCompound - 1);
}

void CMaterialsDatabase::ShiftCompoundUp(const std::string& _sCompoundUniqueKey)
{
	ShiftCompoundUp(GetCompoundIndex(_sCompoundUniqueKey));
}

void CMaterialsDatabase::ShiftCompoundDown(size_t _iCompound)
{
	if ((_iCompound < m_vCompounds.size()) && (_iCompound != (m_vCompounds.size() - 1)))
		std::iter_swap(m_vCompounds.begin() + _iCompound, m_vCompounds.begin() + _iCompound + 1);
}

void CMaterialsDatabase::ShiftCompoundDown(const std::string& _sCompoundUniqueKey)
{
	ShiftCompoundDown(GetCompoundIndex(_sCompoundUniqueKey));
}

size_t CMaterialsDatabase::GetCompoundIndex(const std::string& _sCompoundUniqueKey) const
{
	for (size_t i = 0; i < m_vCompounds.size(); ++i)
		if (m_vCompounds[i].GetKey() == _sCompoundUniqueKey)
			return i;
	return -1; // will be implicitly converted to size_t::max
}

CCompound* CMaterialsDatabase::GetCompound(size_t _iCompound)
{
	return const_cast<CCompound*>(static_cast<const CMaterialsDatabase&>(*this).GetCompound(_iCompound));
}

const CCompound* CMaterialsDatabase::GetCompound(size_t _iCompound) const
{
	if (_iCompound < m_vCompounds.size())
		return &m_vCompounds[_iCompound];
	return nullptr;
}

CCompound* CMaterialsDatabase::GetCompound(const std::string& _sCompoundUniqueKey)
{
	return const_cast<CCompound*>(static_cast<const CMaterialsDatabase&>(*this).GetCompound(_sCompoundUniqueKey));
}

const CCompound* CMaterialsDatabase::GetCompound(const std::string& _sCompoundUniqueKey) const
{
	for(const auto& c : m_vCompounds)
		if (c.GetKey() == _sCompoundUniqueKey)
			return &c;
	return nullptr;
}

CCompound* CMaterialsDatabase::GetCompoundByName(const std::string& _sCompoundName)
{
	return const_cast<CCompound*>(static_cast<const CMaterialsDatabase&>(*this).GetCompoundByName(_sCompoundName));
}

const CCompound* CMaterialsDatabase::GetCompoundByName(const std::string& _sCompoundName) const
{
	for (const auto& c : m_vCompounds)
		if (c.GetName() == _sCompoundName)
			return &c;
	return nullptr;
}

double CMaterialsDatabase::GetConstPropertyValue(const std::string& _sCompoundUniqueKey, ECompoundConstProperties _nConstPropType) const
{
	if(const CCompound *comp = GetCompound(_sCompoundUniqueKey))
		return comp->GetConstPropertyValue(_nConstPropType);
	return 0;
}

double CMaterialsDatabase::GetTPPropertyValue(const std::string& _sCompoundUniqueKey, ECompoundTPProperties _nTPPropType, double _dT, double _dP) const
{
	if (const CCompound *comp = GetCompound(_sCompoundUniqueKey))
		return comp->GetTPPropertyValue(_nTPPropType, _dT, _dP);
	return 0;
}

double CMaterialsDatabase::GetInteractionPropertyValue(const std::string& _sCompoundUniqueKey1, const std::string& _sCompoundUniqueKey2, EInteractionProperties _nInterPropType, double _dT, double _dP) const
{
	if (const CInteraction* inter = GetInteraction(_sCompoundUniqueKey1, _sCompoundUniqueKey2))
		return inter->GetPropertyValue(_nInterPropType, _dT, _dP);
	return 0;
}

SInterval CMaterialsDatabase::GetTPPropertyTInterval(const std::string& _sCompoundUniqueKey, ECompoundTPProperties _nTPPropType) const
{
	if (const CCompound *comp = GetCompound(_sCompoundUniqueKey))
		if (const CTPDProperty *prop = comp->GetTPProperty(_nTPPropType))
			return prop->GetTInterval();
	return SInterval{ -1, -1 };
}

SInterval CMaterialsDatabase::GetTPPropertyPInterval(const std::string& _sCompoundUniqueKey, ECompoundTPProperties _nTPPropType) const
{
	if (const CCompound *comp = GetCompound(_sCompoundUniqueKey))
		if (const CTPDProperty *prop = comp->GetTPProperty(_nTPPropType))
			return prop->GetPInterval();
	return SInterval{ -1, -1 };
}

size_t CMaterialsDatabase::InteractionsNumber() const
{
	return m_vInteractions.size();
}

size_t CMaterialsDatabase::GetInteractionIndex(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2) const
{
	for (size_t i = 0; i < m_vInteractions.size(); ++i)
		if (m_vInteractions[i].IsBetween(_sCompoundKey1, _sCompoundKey2))
			return i;
	return -1; // will be implicitly converted to size_t::max
}

CInteraction* CMaterialsDatabase::GetInteraction(size_t _iInteraction)
{
	return const_cast<CInteraction*>(static_cast<const CMaterialsDatabase&>(*this).GetInteraction(_iInteraction));
}

const CInteraction* CMaterialsDatabase::GetInteraction(size_t _iInteraction) const
{
	if (_iInteraction < m_vInteractions.size())
		return &m_vInteractions[_iInteraction];
	return nullptr;
}

CInteraction* CMaterialsDatabase::GetInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2)
{
	return const_cast<CInteraction*>(static_cast<const CMaterialsDatabase&>(*this).GetInteraction(_sCompoundKey1, _sCompoundKey2));
}

const CInteraction* CMaterialsDatabase::GetInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2) const
{
	return GetInteraction(GetInteractionIndex(_sCompoundKey1, _sCompoundKey2));
}

double CMaterialsDatabase::GetInteractionValue(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2, EInteractionProperties _nInterPropType, double _dT, double _dP) const
{
	for (const auto& i : m_vInteractions)
		if (i.IsBetween(_sCompoundKey1, _sCompoundKey2))
			return i.GetPropertyValue(_nInterPropType, _dT, _dP);
	return 0;
}

std::vector<std::string> CMaterialsDatabase::GetCompoundsKeys() const
{
	std::vector<std::string> vKeys;
	for (const auto& c : m_vCompounds)
		vKeys.push_back(c.GetKey());
	return vKeys;
}

CInteraction* CMaterialsDatabase::AddInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2)
{
	// check existence
	for (auto& i : m_vInteractions)
		if (i.IsBetween(_sCompoundKey1, _sCompoundKey2))
			return &i;

	// create new interaction
	m_vInteractions.emplace_back(activeInterProperties, _sCompoundKey1, _sCompoundKey2);
	return &m_vInteractions.back();
}

void CMaterialsDatabase::RemoveInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2)
{
	RemoveInteraction(GetInteractionIndex(_sCompoundKey1, _sCompoundKey2));
}

void CMaterialsDatabase::RemoveInteraction(size_t _iInteraction)
{
	if (_iInteraction < m_vInteractions.size())
		m_vInteractions.erase(m_vInteractions.begin() + _iInteraction);
}

void CMaterialsDatabase::ConformInteractionsAdd(const std::string& _sCompoundKey)
{
	// add necessary default interactions
	for (const auto& i : m_vCompounds)
		AddInteraction(_sCompoundKey, i.GetKey());
}

void CMaterialsDatabase::ConformInteractionsRemove(const std::string& _sCompoundKey)
{
	// remove unnecessary interactions
	unsigned i = 0;
	while (i < m_vInteractions.size())
		if ((m_vInteractions[i].GetKey1() == _sCompoundKey) || (m_vInteractions[i].GetKey2() == _sCompoundKey))
			RemoveInteraction(i);
		else
			++i;
}

std::string CMaterialsDatabase::Comment(const std::string& _s)
{
	return " \t" + StrConst::COMMENT_SYMBOL + " " + _s;
}
