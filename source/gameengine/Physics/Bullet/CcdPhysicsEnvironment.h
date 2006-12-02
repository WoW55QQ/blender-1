/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef CCDPHYSICSENVIRONMENT
#define CCDPHYSICSENVIRONMENT

#include "PHY_IPhysicsEnvironment.h"
#include <vector>
class CcdPhysicsController;
#include "LinearMath/btVector3.h"
#include "LinearMath/btTransform.h"




class btTypedConstraint;
class btSimulationIslandManager;
class btCollisionDispatcher;
class btDispatcher;
//#include "btBroadphaseInterface.h"

//switch on/off new vehicle support
#define NEW_BULLET_VEHICLE_SUPPORT 1

#include "BulletDynamics/ConstraintSolver/btContactSolverInfo.h"

class WrapperVehicle;
class btPersistentManifold;
class btBroadphaseInterface;
class btOverlappingPairCache;
class btIDebugDraw;
class PHY_IVehicle;

/// CcdPhysicsEnvironment is an experimental mainloop for physics simulation using optional continuous collision detection.
/// Physics Environment takes care of stepping the simulation and is a container for physics entities.
/// It stores rigidbodies,constraints, materials etc.
/// A derived class may be able to 'construct' entities by loading and/or converting
class CcdPhysicsEnvironment : public PHY_IPhysicsEnvironment
{
	btVector3 m_gravity;
	
	

protected:
	btIDebugDraw*	m_debugDrawer;
	//solver iterations
	int	m_numIterations;
	
	//timestep subdivisions
	int	m_numTimeSubSteps;


	int	m_ccdMode;
	int	m_solverType;
	int	m_profileTimings;
	bool m_enableSatCollisionDetection;

	btContactSolverInfo	m_solverInfo;
	

	public:
		CcdPhysicsEnvironment(btDispatcher* dispatcher=0, btOverlappingPairCache* pairCache=0);

		virtual		~CcdPhysicsEnvironment();

		/////////////////////////////////////
		//PHY_IPhysicsEnvironment interface
		/////////////////////////////////////

		/// Perform an integration step of duration 'timeStep'.

		virtual void setDebugDrawer(btIDebugDraw* debugDrawer);

		virtual void		setNumIterations(int numIter);
		virtual void		setNumTimeSubSteps(int numTimeSubSteps)
		{
			m_numTimeSubSteps = numTimeSubSteps;
		}
		virtual void		setDeactivationTime(float dTime);
		virtual	void		setDeactivationLinearTreshold(float linTresh) ;
		virtual	void		setDeactivationAngularTreshold(float angTresh) ;
		virtual void		setContactBreakingTreshold(float contactBreakingTreshold) ;
		virtual void		setCcdMode(int ccdMode);
		virtual void		setSolverType(int solverType);
		virtual void		setSolverSorConstant(float sor);
		virtual void		setSolverTau(float tau);
		virtual void		setSolverDamping(float damping);
		virtual void		setLinearAirDamping(float damping);
		virtual void		setUseEpa(bool epa) ;

		virtual	void		beginFrame();
		virtual void		endFrame() {};
		/// Perform an integration step of duration 'timeStep'.
		virtual	bool		proceedDeltaTime(double curTime,float timeStep);
//		virtual bool		proceedDeltaTimeOneStep(float timeStep);

		virtual	void		setFixedTimeStep(bool useFixedTimeStep,float fixedTimeStep){};
		//returns 0.f if no fixed timestep is used

		virtual	float		getFixedTimeStep(){ return 0.f;};

		virtual void		setDebugMode(int debugMode);

		virtual	void		setGravity(float x,float y,float z);

		virtual int			createConstraint(class PHY_IPhysicsController* ctrl,class PHY_IPhysicsController* ctrl2,PHY_ConstraintType type,
			float pivotX,float pivotY,float pivotZ,
			float axisX,float axisY,float axisZ);


		//Following the COLLADA physics specification for constraints
		virtual int			createUniversalD6Constraint(
		class PHY_IPhysicsController* ctrlRef,class PHY_IPhysicsController* ctrlOther,
			btTransform& localAttachmentFrameRef,
			btTransform& localAttachmentOther,
			const btVector3& linearMinLimits,
			const btVector3& linearMaxLimits,
			const btVector3& angularMinLimits,
			const btVector3& angularMaxLimits
			);

		virtual void	setConstraintParam(int constraintId,int param,float value,float value1);

	    virtual void		removeConstraint(int	constraintid);

		virtual float		getAppliedImpulse(int	constraintid);


		virtual void	CallbackTriggers();


#ifdef NEW_BULLET_VEHICLE_SUPPORT
		//complex constraint for vehicles
		virtual PHY_IVehicle*	getVehicleConstraint(int constraintId);
#else
		virtual class PHY_IVehicle*	getVehicleConstraint(int constraintId)
		{
			return 0;
		}
#endif //NEW_BULLET_VEHICLE_SUPPORT

		btTypedConstraint*	getConstraintById(int constraintId);

		virtual PHY_IPhysicsController* rayTest(PHY_IPhysicsController* ignoreClient, float fromX,float fromY,float fromZ, float toX,float toY,float toZ, 
										float& hitX,float& hitY,float& hitZ,float& normalX,float& normalY,float& normalZ);


		//Methods for gamelogic collision/physics callbacks
		virtual void addSensor(PHY_IPhysicsController* ctrl);
		virtual void removeSensor(PHY_IPhysicsController* ctrl);
		virtual void addTouchCallback(int response_class, PHY_ResponseCallback callback, void *user);
		virtual void requestCollisionCallback(PHY_IPhysicsController* ctrl);
		virtual void removeCollisionCallback(PHY_IPhysicsController* ctrl);

		virtual PHY_IPhysicsController*	CreateSphereController(float radius,const PHY__Vector3& position);
		virtual PHY_IPhysicsController* CreateConeController(float coneradius,float coneheight);
	

		virtual int	getNumContactPoints();

		virtual void getContactPoint(int i,float& hitX,float& hitY,float& hitZ,float& normalX,float& normalY,float& normalZ);

		//////////////////////
		//CcdPhysicsEnvironment interface
		////////////////////////
	
		void	addCcdPhysicsController(CcdPhysicsController* ctrl);

		void	removeCcdPhysicsController(CcdPhysicsController* ctrl);

		btBroadphaseInterface*	getBroadphase();

		
		
		

		bool	IsSatCollisionDetectionEnabled() const
		{
			return m_enableSatCollisionDetection;
		}

		void	EnableSatCollisionDetection(bool enableSat)
		{
			m_enableSatCollisionDetection = enableSat;
		}

	
		int	GetNumControllers();

		CcdPhysicsController* GetPhysicsController( int index);

		

		const btPersistentManifold*	GetManifold(int index) const;

	
		void	SyncMotionStates(float timeStep);

		
	
		class btConstraintSolver*	GetConstraintSolver();

	protected:
		
		

		
		std::vector<CcdPhysicsController*> m_controllers;
		
		std::vector<CcdPhysicsController*> m_triggerControllers;

		PHY_ResponseCallback	m_triggerCallbacks[PHY_NUM_RESPONSE];
		void*			m_triggerCallbacksUserPtrs[PHY_NUM_RESPONSE];
		
		std::vector<WrapperVehicle*>	m_wrapperVehicles;

		class	btDynamicsWorld*	m_dynamicsWorld;
		
		class btConstraintSolver*	m_solver;

		bool	m_scalingPropagated;

		

};

#endif //CCDPHYSICSENVIRONMENT
