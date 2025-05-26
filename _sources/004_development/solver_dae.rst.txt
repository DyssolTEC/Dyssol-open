.. _sec.development.api.solver_dae:

DAE Systems
===========

In Dyssol, you can solve systems of :abbr:`DAE (Differential-algebraic equations)` automatically. In this case, the unit should contain one or several additional objects of class ``CDAEModel``. This class is used to describe :abbr:`DAE (Differential-algebraic equations)` systems and can be automatically solved with class ``CDAESolver``. 

|

DAE model
---------

.. code-block:: cpp

	CDAEModel()

Basic **constructor**. Creates an empty DAE model.

|

Variables
"""""""""

.. _label-AddDAEVariable:

.. code-block:: cpp

	unsigned AddDAEVariable(bool _bIsDifferentiable, double _dVariableInit, double _dDerivativeInit, double _dConstraint)
	
Adds new algebraic (``_bIsDifferentiable = false``) or differential (``_bIsDifferentiable = true``) variable with initial values ``_dVariableInit`` and ``_dDerivativeInit``. 

Should be called in function ``Initialize(Time)`` of the unit. Returns unique index of added variable. 

``_dConstraint`` sets the constraint for the variable, different values of ``_dConstraint`` are defined as follows:

	-	0.0: no constraint
	
	-	1.0: Variable ≥ 0.0
	
	-	−1.0: Variable ≤ 0.0
	
	-	2.0: Variable > 0.0
	
	-	−2.0: Variable < 0.0

|

.. code-block:: cpp

	unsigned GetVariablesNumber()
	
Returns current number of defined variables. 

|

.. code-block:: cpp

	void ClearVariables()

Removes all defined variables from the model.

|

Tolerance
"""""""""

.. _label-setTol:

.. code-block:: cpp

	void SetTolerance(double _dRTol, double _dATol)

Sets values of relative and absolute tolerances for all variables. 

|

.. code-block:: cpp

	void SetTolerance(double _dRTol, std::vector<double> &_vATol)

Sets value of relative tolerance for all variables and absolute tolerances for each variable separately.

|

.. code-block:: cpp

	double GetRTol()

Returns current relative tolerance. 

|

.. code-block:: cpp

	double GetATol(unsigned _dIndex)

Returns current absolute tolerance for specified variable. 

|

Virtual functions
"""""""""""""""""

.. _label-CalculateResiduals:

.. code-block:: cpp

	virtual void CalculateResiduals(double _dTime, double *_pVars, double *_pDerivs, double *_pRes, void *_pUserData)
	
Computes the problem residual for given values of the independent variable ``_dTime``, state vector ``_pVars``, and derivatives ``_pDerivs``. Here the :abbr:`DAE (Differential-algebraic equations)` system should be specified in implicit form. Function will be called by solver automatically.

	- ``_dTime``: current value of the independent variable :math:`t`.

	- ``_pVars``: pointer to an array of the dependent variables, :math:`y(t)`.

	- ``_pDerivs``: pointer to an array of derivatives :math:`y'(t)`.

	- ``_pRes``: output residual vector :math:`F(t, y, y')`.

	- ``_pUserData``: pointer to user's data. Is used to provide access from this function to unit’s data.

|

.. _label-ResultsHandler:

.. code-block:: cpp

	virtual void ResultsHandler(double _dTime, double *_pVars, double *_pDerivs, void *_pUserData)
	
Processing the results returned by the solver at each calculated step. Called by solver every time when the solution in new time point is ready.

	- ``_dTime``: current value of the independent variable :math:`t`.

	- ``_pVars``: current values of the dependent variables, :math:`y(t)`.

	- ``_pDerivs``: current values of derivatives :math:`y'(t)`.

	- ``_pUserData``: pointer to user's data. Is used to provide access from this function to unit’s data.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void Clear()
	
Removes all data from the model. 

|

.. code-block:: cpp

	void SetUserData(void *_pUserData)

Set pointer to user’s data. This data will be returned in overloaded functions :ref:`CalculateResiduals <label-CalculateResiduals>` and :ref:`ResultsHandler <label-ResultsHandler>`. Usually is used to provide access from these functions to unit’s data.

|

.. _label-DAEsolver:

DAE solver
----------

.. code-block:: cpp

	CDAESolver()

Basic **constructor**. Creates an empty solver.

|

Model
"""""

.. _label-setModel:

.. code-block:: cpp

	bool SetModel(CDAEModel *_pModel)

Sets model to a solver. Should be called in function :ref:`Initialize <label-DynamicUnitInitialize>` of the unit. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetMaxStep()

Sets maximum time step for solver. Should be used in :ref:`Initialize <label-DynamicUnitInitialize>` before the function :ref:`SetModel <label-setModel>`.

|

.. _label-Calculate:

.. code-block:: cpp

	bool Calculate(double _dTime, unsigned _nModel = KIN_NONE)

Solves problem on a given time point ``_dTime``. Should be called in function :ref:`Simulate <label-DynamicUnitSimulate>` of the unit. 

Default value of ``_nModel`` is ``KIN_NONE``, a Newton-based iteration method. Other available models can be found in declaration for KINSOL parameters in the ``*.cpp`` file, including: ``KIN_FP`` (fixed point), ``KIN_LINESEARCH`` and ``KIN_PICARD``.


Returns ``false`` on error.

|

.. code-block:: cpp

	bool Calculate(double _dStartTime, double _dEndTime)

Solves problem on a given time interval. Should be called in function :ref:`Simulate <label-DynamicUnitSimulate>` of the unit. Returns ``false`` on error.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void SaveState()

Saves current state of the solver. Should be called in function :ref:`SaveState <sec.development.api.class_baseunit>` of the unit.

|

.. code-block:: cpp

	void LoadState()

Loads last saved state of the solver. Should be called in function :ref:`LoadState <sec.development.api.class_baseunit>` of the unit.

|

.. code-block:: cpp

	std::string GetError()

Returns error’s description. Can be called if function :ref:`SetModel <label-setModel>` or :ref:`Calculate <label-Calculate>` returns ``false``.

|

Application example
-------------------

Assume that you are going to solve a dynamic :abbr:`DAE (Differential-algebraic equations)` system,

.. math::

	\begin{cases}
		\dfrac{dx}{dt} = -0.04 \cdot x + 10^4 \cdot y \cdot z &x(0) = 0.04 
		
		\dfrac{dy}{dt} = 0.04\cdot x - 10^4 \cdot y \cdot z - 3 \cdot 10^7 \cdot y^2 &y(0) = -0.04
		
		x + y + z = 1
	\end{cases}

where :math:`x`, :math:`y`, :math:`z` are fractions of solid, liquid and vapor phases of the output stream of the unit.

Now you want to develope a new unit for for automatic calculation of this :abbr:`DAE (Differential-algebraic equations)` by using built-in solvers of Dyssol, just do the following steps:

1.	Add a new template unit to your solution and name it ``DynamicUnitWithSolver``, please refer to :ref:`label-unitDev`. 

2.	In file ``Unit.h``, there is already a class of :abbr:`DAE (Differential-algebraic equations)` model ``CMyDAEModel`` defined with:

	- two functions, which must be overridden (:ref:`CalculateResiduals <label-CalculateResiduals>` and :ref:`ResultsHandler <label-ResultsHandler>`) and
	
	- a class of unit ``CUnit`` with two additional variables (for :abbr:`DAE (Differential-algebraic equations)` model ``CMyDAEModel m_Model``, for :abbr:`DAE (Differential-algebraic equations)` solver ``CDAESolver m_Solver``). 

	Add three variables in class ``CMyDAEModel`` to store indices for :math:`x`, :math:`y` and :math:`z` variables, name them ``m_nS``, ``m_nL`` and ``m_nV`` respectively. 
	
	After adding description of class ``CMyDAEModel``, your code should look like this:
	
	.. code-block:: cpp
	
		class CMyDAEModel : public CDAEModel
		{		
		public:		
			unsigned m_nS; //solid fraction			
			unsigned m_nL; //liquid fraction			
			unsigned m_nV; //vapor fraction
			
		public:		
			void CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData);			
			void ResultsHandler(double _dTime, double* _pVars, double* _pDers, void *_pUserData);
		};

3.	Setup unit’s basic info (name, author’s name, unique ID, ports) as described in :ref:`label-unitDev`.Then provide model with pointer to your unit, in order to have an access to the unit from functions of the model. Now your unit’s constructor should look like this:

	.. code-block:: cpp
	
		CUnit::CUnit()
		{
			// Unit basic information
			m_sUnitName = "DummyUnit4";
			m_sAuthorName = "Author";
			m_sUniqueID = "344BCC0048AA4c3a9117F20A9F8AF9A8"; //an example ID

			// Add ports for in- and outlets
			AddPort("InPort", INPUT_PORT);
			AddPort("OutPort", OUTPUT_PORT);

			// Set this unit as user data of the model applied 
			m_Model.SetUserData(this);
		}

4.	Implement function ``Initialize`` of the unit: 

	Since function ``Initialize`` is called every time when simulation starts, all variables must be previously removed from the model by calling ``ClearVariables``. 

	.. code-block:: cpp

		m_Model.ClearVariables();
	
	Now you can add 3 variables with specified initial conditions to the model (using function :ref:`AddDAEVariable  <label-AddDAEVariable>`) according to the equation system. Use phase fractions of the input stream as initials. 

	Set tolerances to the model using function :ref:`SetTolerance <label-setTol>`. As tolerances for the model, global tolerances of the system can be used. 

	.. code-block:: cpp

		m_Model.SetTolerance( GetRelTolerance(), GetAbsTolerance() );

	Now, you can connect your model to the solver by calling function :ref:`SetModel <label-setModel>`. To receive errors from solver, connect it to the global errors handling procedure. 

	.. code-block:: cpp

		if( !m_Solver.SetModel(&m_Model) )
			RaiseError(m_Solver.GetError());
				
	Your code should look like this after following all steps above:

	.. code-block:: cpp

		void CUnit::Initialize(double _dTime)
		{
			// Get pointer to inlet stream
			CMaterialStream* pInpStream = GetPortStream("InPort");	
			
			// Clear previous state variables in model
			m_Model.ClearVariables();
			
			// Add state variables to model
			double dSFr = pInpStream->GetSinglePhaseProp(_dTime, FRACTION, SOA_SOLID);
			double dLFr = pInpStream->GetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID);
			double dVFr = pInpStream->GetSinglePhaseProp(_dTime, FRACTION, SOA_VAPOR);
			m_Model.m_nS = m_Model.AddDAEVariable(true, dSFr, 0.04, 1.0);
			m_Model.m_nL = m_Model.AddDAEVariable(true, dLFr, -0.04, 1.0);
			m_Model.m_nV = m_Model.AddDAEVariable(false, dVFr, 0, 1.0);
			
			// Set tolerance to model
			m_Model.SetTolerance( GetRelTolerance(), GetAbsTolerance() );
			
			// Set model to solver
			if( !m_Solver.SetModel(&m_Model) )
				RaiseError(m_Solver.GetError());
		}
			
5.	Connect solver to a system saving / loading procedure in functions ``SaveState()`` and ``LoadState()`` of the unit:

	.. code-block:: cpp

		void CUnit::SaveState()
		{
			m_Solver.SaveState();
		}

		void CUnit::LoadState()
		{
			m_Solver.LoadState();
		}

6.	In function ``Simulate`` of the unit, calculation procedure should be run by calling function ``Calculate`` of the solver. Additionally, solver can be connected to the system’s errors handling procedure to receive possible errors during the calculation. Unit’s ``Simulate`` function after that must look as follows:

	.. code-block:: cpp

		void CUnit::Simulate(double _dStartTime, double _dEndTime)
		{	
			// Get pointers to inlet and outlet streams
			CMaterialStream *pInputStream = GetPortStream("InPort");
			CMaterialStream *pOutputStream = GetPortStream("OutPort");

			pOutputStream->RemoveTimePointsAfter(_dStartTime, true);
			pOutputStream->CopyFromStream(pInputStream, _dStartTime); // Copy the inlet stream information to outlet stream
			
			// Run solver and check if errors take place
			if( !m_Solver.Calculate(_dStartTime, _dEndTime) )
				RaiseError( m_Solver.GetError() );
		}

7.	In function ``CalculateResiduals``, :abbr:`DAE (Differential-algebraic equations)` system in implicit form should be described. According to the given equation system
	
	.. math::

		\begin{cases}
			\dfrac{dx}{dt} = -0.04 \cdot x + 10^4 \cdot y \cdot z &x(0) = 0.04 
			
			\dfrac{dy}{dt} = 0.04\cdot x - 10^4 \cdot y \cdot z - 3 \cdot 10^7 \cdot y^2 &y(0) = -0.04
			
			x + y + z = 1
		\end{cases}

	and the definition of residual ``_pRes`` (difference between new and old values from last calculation), the equaitons are expressed below. Here the values are arranged in vector ``_pVars`` according to sequence :math:`[x, y, z]`, in ``_pDers`` according to :math:`[dx, dy]`.
		
	.. code-block:: cpp

		void CMyDAEModel::CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData)
		{
			_pRes[0] = _pDers[0] - (-0.04*_pVars[0] + 1.0e4*_pVars[1]*_pVars[2]);
			_pRes[1] = _pDers[1] - ( 0.04*_pVars[0] - 1.0e4*_pVars[1]*_pVars[2] - 3.0e7*_pVars[1]*_pVars[1] );
			_pRes[2] = _pVars[0] + _pVars[1] + _pVars[2] - 1;
		}

8.	Last step is handling of results from the solver (``CMyDAEModel::ResultsHandler``). Calculated fractions can be set here to the output stream of the unit. To access to the unit’s data, use the pointer ``_pUserData`` defined previously:

	.. code-block:: cpp

		void CMyDAEModel::ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void *_pUserData)
		{
			// Get pointers to streams
			CUnit *unit = static_cast<CUnit*>(_pUserData);
			CMaterialStream *pStream = static_cast<CMaterialStream*>(unit->GetPortStream("OutPort"));
			
			// Add time point to outlet stream
			pStream->AddTimePoint(_dTime);
			
			// Set calculated results to corresponding phases
			pStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_SOLID, _pVars[0]);
			pStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID, _pVars[1]);
			pStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_VAPOR, _pVars[2]);
		} 

|
