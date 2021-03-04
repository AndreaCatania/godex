#pragma once

#include "../../components/component.h"
#include "../../storage/dense_vector_storage.h"
#include "../../storage/shared_steady_storage.h"
#include "../../storage/steady_storage.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btMotionState.h>

class btCollisionShape;

/// This is an optional component, that allow to specify a specific physics
/// `Space` where the `Entity` is put.
/// When this component is not set, the `Entity` is put to the default main
/// space.
///
/// Note, you have at max 4 spaces, and it's unlikely that you need more than 1.
struct BtSpaceMarker {
	COMPONENT(BtSpaceMarker, DenseVectorStorage)

	static void _bind_methods();

	uint32_t space_id = 0;
};

/// This class is responsible to move a kinematic body and tell when a body
/// transform changed
/// DOC:
/// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/MotionStates#What.27s_a_MotionState.3F
class GodexMotionState : public btMotionState {
	/// This data is used to store the new world position for kinematic body
	btTransform bodyKinematicWorldTransf;
	/// This data is used to store last world position
	btTransform bodyCurrentWorldTransform;

public:
	GodexMotionState() :
			bodyKinematicWorldTransf(btMatrix3x3(1., 0., 0., 0., 1., 0., 0., 0., 1.), btVector3(0., 0., 0.)),
			bodyCurrentWorldTransform(btMatrix3x3(1., 0., 0., 0., 1., 0., 0., 0., 1.), btVector3(0., 0., 0.)) {}

	/// IMPORTANT DON'T USE THIS FUNCTION TO KNOW THE CURRENT BODY TRANSFORM
	/// This class is used internally by Bullet
	/// Use GodotMotionState::getCurrentWorldTransform to know current position
	///
	/// This function is used by Bullet to get the position of object in the world
	/// if the body is kinematic Bullet will move the object to this location
	/// if the body is static Bullet doesn't move at all
	virtual void getWorldTransform(btTransform &worldTrans) const override {
		worldTrans = bodyKinematicWorldTransf;
	}

	/// IMPORTANT: to move the body use: moveBody
	/// IMPORTANT: DON'T CALL THIS FUNCTION, IT IS CALLED BY BULLET TO UPDATE RENDERING ENGINE
	///
	/// This function is called each time by Bullet and set the current position of body
	/// inside the physics world.
	/// Don't allow Godot rendering scene takes world transform from this object because
	/// the correct transform is set by Bullet only after the last step when there are sub steps
	/// This function must update Godot transform rendering scene for this object.
	virtual void setWorldTransform(const btTransform &worldTrans) override {
		bodyCurrentWorldTransform = worldTrans;

		//owner->notify_transform_changed();
	}

public:
	/// Use this function to move kinematic body
	/// -- or set initial transform before body creation.
	void moveBody(const btTransform &newWorldTransform) {
		bodyKinematicWorldTransf = newWorldTransform;
	}

	/// It returns the current body transform from last Bullet update
	const btTransform &getCurrentWorldTransform() const {
		return bodyCurrentWorldTransform;
	}
};

/// This Component represent a Bullet Physics RigidBody.
/// The RigidBody can be STATIC, DYNAMIC, KINEMATIC.
struct BtRigidBody {
	COMPONENT(BtRigidBody, SteadyStorage)

	enum RigidMode {
		RIGID_MODE_DYNAMIC,
		RIGID_MODE_CHARACTER,
		RIGID_MODE_KINEMATIC,
		RIGID_MODE_STATIC
	};

	static void _bind_methods();
	static void _get_storage_config(Dictionary &r_config);

private:
	real_t mass = 1.0;
	GodexMotionState motion_state;
	btRigidBody body = btRigidBody(0.0, &motion_state, nullptr, btVector3(0.0, 0.0, 0.0));

	/// This point to the assigned share. When this is not `nullptr` always
	/// assume it exists. If the shape is destroyed, this is cleared.
	btCollisionShape *main_shape = nullptr;

public:
	void script_set_body_mode(uint32_t p_mode);
	void set_body_mode(RigidMode p_mode);
	RigidMode get_body_mode() const;

	void set_mass(real_t p_mass);
	real_t get_mass() const;
};
