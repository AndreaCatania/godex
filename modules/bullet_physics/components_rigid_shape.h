#pragma once

#include "../../components/component.h"
#include "../../storage/shared_steady_storage.h"
#include "../../storage/steady_storage.h"
#include <btBulletCollisionCommon.h>

struct BtRigidShape {
private:
	btCollisionShape *shape = nullptr;

public:
	BtRigidShape(btCollisionShape *p_shape) :
			shape(p_shape) {}

	btCollisionShape *get_shape() { return shape; }
	const btCollisionShape *get_shape() const { return shape; }
};

struct BtShapeBox : public BtRigidShape {
	COMPONENT_CUSTOM_CONSTRUCTOR(BtShapeBox, SteadyStorage) // TODO Please used the SharedComponent instead.

	static void _bind_methods();
	static void _get_storage_config(Dictionary &r_config);

	btBoxShape box = btBoxShape(btVector3(1.0, 1.0, 1.0));

	BtShapeBox() :
			BtRigidShape(&box) {}

	void set_half_extents(const Vector3 &p_half_extends);
	Vector3 get_half_extents() const;

	void set_margin(real_t p_margin);
	real_t get_margin() const;
};