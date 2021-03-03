#include "space_databag.h"

#include "core/config/project_settings.h"
#include "modules/bullet/godot_collision_configuration.h"
#include "modules/bullet/godot_collision_dispatcher.h"
#include "modules/bullet/godot_result_callbacks.h"
#include <BulletCollision/BroadphaseCollision/btBroadphaseProxy.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h>
#include <BulletCollision/NarrowPhaseCollision/btPointCollector.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>

btScalar calculate_godot_combined_restitution(const btCollisionObject *body0, const btCollisionObject *body1) {
	return CLAMP(body0->getRestitution() + body1->getRestitution(), 0, 1);
}

btScalar calculate_godot_combined_friction(const btCollisionObject *body0, const btCollisionObject *body1) {
	return ABS(MIN(body0->getFriction(), body1->getFriction()));
}

void on_post_tick_callback(btDynamicsWorld *p_dynamics_world, btScalar p_delta) {
	//BtSpace *space = static_cast<BtSpace *>(p_dynamics_world->getWorldUserInfo());
}

BtSpaces::BtSpaces() {
	// Always init the space 0, which is the default one.
	init_space(SPACE_0, GLOBAL_DEF("physics/3d/active_soft_world", true));
	CRASH_COND_MSG(is_space_initialized(SPACE_0) == false, "At this point the space 0 is expected to be initialized.");
}

BtSpaces::~BtSpaces() {
	// Free all the spaces.
	for (uint32_t i = 0; i < SPACE_MAX; i += 1) {
		free_space(static_cast<SpaceID>(i));
	}
}

bool BtSpaces::is_space_initialized(SpaceID p_id) const {
	return spaces[p_id].broadphase != nullptr;
}

void BtSpaces::init_space(SpaceID p_id, bool p_soft_world) {
	ERR_FAIL_COND_MSG(is_space_initialized(p_id), "This space " + itos(p_id) + " is already initialized");

	void *world_mem;
	if (p_soft_world) {
		world_mem = malloc(sizeof(btSoftRigidDynamicsWorld));
		spaces[p_id].collision_configuration = memnew(GodotSoftCollisionConfiguration(
				static_cast<btDiscreteDynamicsWorld *>(world_mem)));
	} else {
		world_mem = malloc(sizeof(btDiscreteDynamicsWorld));
		spaces[p_id].collision_configuration = memnew(GodotCollisionConfiguration(
				static_cast<btDiscreteDynamicsWorld *>(world_mem)));
	}

	spaces[p_id].dispatcher = memnew(GodotCollisionDispatcher(spaces[p_id].collision_configuration));
	spaces[p_id].broadphase = memnew(btDbvtBroadphase);
	spaces[p_id].solver = new btSequentialImpulseConstraintSolver;

	if (p_soft_world) {
		spaces[p_id].dynamics_world =
				new (spaces[p_id].dynamics_world) btSoftRigidDynamicsWorld(
						spaces[p_id].dispatcher,
						spaces[p_id].broadphase,
						spaces[p_id].solver,
						spaces[p_id].collision_configuration);
		spaces[p_id].soft_body_world_info = memnew(btSoftBodyWorldInfo);
	} else {
		spaces[p_id].dynamics_world =
				new (spaces[p_id].dynamics_world) btDiscreteDynamicsWorld(
						spaces[p_id].dispatcher,
						spaces[p_id].broadphase,
						spaces[p_id].solver,
						spaces[p_id].collision_configuration);
	}

	// Set the space as User Info pointer, so I can access this space within
	// bullet callbacks.
	// Note, the `spaces` memory location doesn't change, so it's fine pass the
	// pointer.
	spaces[p_id].dynamics_world->setWorldUserInfo(spaces + p_id);

	// Set global callbacks.
	gCalculateCombinedRestitutionCallback = &calculate_godot_combined_restitution;
	gCalculateCombinedFrictionCallback = &calculate_godot_combined_friction;
	gContactAddedCallback = &godotContactAddedCallback;

	// Setup the world callbacks.
	spaces[p_id].ghost_pair_callback = memnew(btGhostPairCallback);
	spaces[p_id].godot_filter_callback = memnew(GodotFilterCallback);

	spaces[p_id].dynamics_world->setInternalTickCallback(on_post_tick_callback, this, false);
	spaces[p_id].dynamics_world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(
			spaces[p_id].ghost_pair_callback);
	spaces[p_id].dynamics_world->getPairCache()->setOverlapFilterCallback(
			spaces[p_id].godot_filter_callback);
}

void BtSpaces::free_space(SpaceID p_id) {
	if (is_space_initialized(p_id)) {
		// Nothing to do.
		return;
	}

	memdelete(spaces[p_id].broadphase);
	spaces[p_id].broadphase = nullptr;

	memdelete(spaces[p_id].collision_configuration);
	spaces[p_id].collision_configuration = nullptr;

	memdelete(spaces[p_id].dispatcher);
	spaces[p_id].dispatcher = nullptr;

	delete spaces[p_id].solver;
	spaces[p_id].solver = nullptr;

	memdelete(spaces[p_id].dynamics_world);
	spaces[p_id].dynamics_world = nullptr;

	memdelete(spaces[p_id].ghost_pair_callback);
	spaces[p_id].ghost_pair_callback = nullptr;

	memdelete(spaces[p_id].godot_filter_callback);
	spaces[p_id].godot_filter_callback = nullptr;
}

BtSpace *BtSpaces::get_space(SpaceID p_space_id) {
	return spaces + p_space_id;
}

const BtSpace *BtSpaces::get_space(SpaceID p_space_id) const {
	return spaces + p_space_id;
}
