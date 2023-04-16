.. _sec.development.api.solver_external:

External solver
===============

As mentioned in section :ref:`label-solverDev`, you can generate your own solvers in Dyssol under class ``CExternalSolver``. 

In Dyssol, external solvers are connected with :ref:`open-source programm packages <label-equationSolvers>`. Thus you can only do some basic operations for these solvers, such as generate new solver, name it, get ID and version, initialize and finalize, etc.

In this section, the functions and variables applied in external solver are introduced.

|

.. code-block:: cpp

	CExternalSolver()
	
Basic **constructor** of the solver. Creates an empty external solver. Called only once when solver is added to the unit.

Internal variables are:

- ``m_solverName``: name of the solver that will be displayed in user interface of Dyssol.

- ``m_authorName``: solver’s author

- ``m_solverUniqueKey``: unique identificator of the solver. Simulation environment distinguishes different solvers with the help of this identificator. You must ensure that ID of your solver is unique. This ID can be created manually or using *GUID-generator* of Visual Studio (*Tools → GUID Genarator*).

- ``m_solverVersion``: current version of the solver.

|

Basic information
-----------------

.. code-block:: cpp

	std::string GetName()

Returns name of the solver.

|

.. code-block:: cpp

	std::string GetUniqueID()
	
Returns string key, which is unique among all solvers.

|

.. code-block:: cpp

	std::string GetAuthorName()

Returns name of the solver's author.

|

.. code-block:: cpp

	unsigned GetVersion()

Returns version of the solver.

|

Virtual functions
-----------------

.. code-block:: cpp

	virtual void Initialize()

Solver‘s initialization. This function is called only once for each simulation during the initialization of unit. Implementation of this function is not obligatory and can be skipped.

|

.. code-block:: cpp

	virtual void Finalize()

Unit‘s finalization. This function is called only once for each simulation during the finalization of unit. Implementation of this function is not obligatory and can be skipped.

|
