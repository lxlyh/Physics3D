#pragma once

#include "math/linalg/vec.h"
#include "math/linalg/mat.h"
#include "math/cframe.h"
#include "math/bounds.h"
#include "math/position.h"
#include "math/globalCFrame.h"

#include "datastructures/unorderedVector.h"
#include "datastructures/iteratorEnd.h"

#include "part.h"
#include "rigidBody.h"
#include "constraints/hardConstraint.h"
#include "constraints/hardPhysicalConnection.h"

typedef Vec3 Vec3Local;
typedef Vec3 Vec3Relative;

class WorldPrototype;

class ConnectedPhysical;
class MotorizedPhysical;

class Physical {
	void makeMainPart(AttachedPart& newMainPart);
protected:
	void updateAttachedPhysicals(double deltaT);
	void translateUnsafeRecursive(const Vec3Fix& translation);

	void setMainPhysicalRecursive(MotorizedPhysical* newMainPhysical);

	// deletes the given physical
	void attachPhysical(Physical* phys, const CFrame& attachment);
	// deletes the given physical
	void attachPhysical(MotorizedPhysical* phys, const CFrame& attachment);

	// deletes the given physical
	void attachPhysical(Physical* phys, HardConstraint* constraint, const CFrame& attachToThis, const CFrame& attachToThat);

	// deletes the given physical
	void attachPhysical(MotorizedPhysical* phys, HardConstraint* constraint, const CFrame& attachToThis, const CFrame& attachToThat);

	void removeChild(ConnectedPhysical* child);
	void detachFromRigidBody(Part* part);
	void detachFromRigidBody(AttachedPart&& part);
	
	/*
		Returns a representation for the motion of the center of mass within this physical

		To get the actual motion compute (result.second / result.first)
	*/
	std::pair<double, Vec3> getMotionOfCenterOfMassInternally() const;


public:
	RigidBody rigidBody;

	MotorizedPhysical* mainPhysical;
	UnorderedVector<ConnectedPhysical> childPhysicals;

	Physical() = default;
	Physical(Part* mainPart, MotorizedPhysical* mainPhysical);
	Physical(RigidBody&& rigidBody, MotorizedPhysical* mainPhysical);

	Physical(Physical&& other) noexcept;
	Physical& operator=(Physical&& other) noexcept;
	Physical(const Physical&) = delete;
	void operator=(const Physical&) = delete;

	void setCFrame(const GlobalCFrame& newCFrame);
	void setPartCFrame(Part* part, const GlobalCFrame& newCFrame);
	inline const GlobalCFrame& getCFrame() const { return rigidBody.getCFrame(); }

	inline Position getPosition() const {
		return getCFrame().getPosition();
	}

	/*
		Returns the motion of this physical attached in it's center of mass

		The motion is oriented globally
	*/
	Motion getMotion() const;

	double getVelocityKineticEnergy() const;
	double getAngularKineticEnergy() const;
	double getKineticEnergy() const;

	void applyDragToPhysical(Vec3 origin, Vec3 drag);
	void applyForceToPhysical(Vec3 origin, Vec3 force);
	void applyImpulseToPhysical(Vec3 origin, Vec3 impulse);

	CFrame getRelativeCFrameToMain() const;
	Vec3 localToMain(const Vec3Local& vec) const;

	void makeMainPart(Part* newMainPart);
	void attachPart(Part* part, const CFrame& attachment);
	void detachPart(Part* part, bool partStaysInWorld);
	void attachPart(Part* part, HardConstraint* constraint, const CFrame& attachToThis, const CFrame& attachToThat);

	size_t getNumberOfPartsInThisAndChildren() const;

	void notifyPartPropertiesChanged(Part* part);
	void notifyPartPropertiesAndBoundsChanged(Part* part, const Bounds& oldBounds);
	void notifyPartStdMoved(Part* oldPartPtr, Part* newPartPtr);

	bool isValid() const;

	bool isMainPhysical() const;
	MotorizedPhysical* makeMainPhysical();

private:
	void detachAllChildPhysicals();
	void detachAllHardConstraintsForSinglePartPhysical(bool alsoDelete);
	void detachChildAndGiveItNewMain(ConnectedPhysical&& formerChild);
	void detachChildPartAndDelete(ConnectedPhysical&& formerChild);
};


class ConnectedPhysical : public Physical {
	friend class Physical;
	friend class MotorizedPhysical;

	void refreshCFrame();
	void refreshCFrameRecursive();
public:
	HardPhysicalConnection connectionToParent;
	Physical* parent;

	ConnectedPhysical() = default;
	ConnectedPhysical(RigidBody&& rigidBody, Physical* parent, HardPhysicalConnection&& connectionToParent);
	ConnectedPhysical(Physical&& phys, Physical* parent, HardPhysicalConnection&& connectionToParent);
	ConnectedPhysical(Physical&& phys, Physical* parent, HardConstraint* constraintWithParent, const CFrame& attachOnThis, const CFrame& attachOnParent);
	
	void setCFrame(const GlobalCFrame& newCFrame);
	CFrame getRelativeCFrameToParent() const;

	/*
		Makes this physical the main Physical
	*/
	void makeMainPhysical();

	Motion getRelativeMotionBetweenParentCOMAndSelfCOM() const;

	bool isValid() const;
};

class MotorizedPhysical : public Physical {
	friend class Physical;
	friend class ConnectedPhysical;
	void rotateAroundCenterOfMassUnsafe(const RotMat3& rotation);
public:
	void refreshPhysicalProperties();
	Vec3 totalForce = Vec3(0.0, 0.0, 0.0);
	Vec3 totalMoment = Vec3(0.0, 0.0, 0.0);

	double totalMass;
	Vec3 totalCenterOfMass;

	WorldPrototype* world = nullptr;
	
	SymmetricMat3 forceResponse;
	SymmetricMat3 momentResponse;

	Motion motionOfCenterOfMass;
	
	explicit MotorizedPhysical(Part* mainPart);
	explicit MotorizedPhysical(RigidBody&& rigidBody);
	explicit MotorizedPhysical(Physical&& movedPhys);

	Position getCenterOfMass() const;
	GlobalCFrame getCenterOfMassCFrame() const;

	void ensureWorld(WorldPrototype* world);

	void update(double deltaT);

	void setCFrame(const GlobalCFrame& newCFrame);
	void rotateAroundCenterOfMass(const RotMat3& rotation);
	void translate(const Vec3& translation);

	void applyForceAtCenterOfMass(Vec3 force);
	void applyForce(Vec3Relative origin, Vec3 force);
	void applyMoment(Vec3 moment);
	void applyImpulseAtCenterOfMass(Vec3 impulse);
	void applyImpulse(Vec3Relative origin, Vec3Relative impulse);
	void applyAngularImpulse(Vec3 angularImpulse);
	void applyDragAtCenterOfMass(Vec3 drag);
	void applyDrag(Vec3Relative origin, Vec3Relative drag);
	void applyAngularDrag(Vec3 angularDrag);

	SymmetricMat3 getResponseMatrix(const Vec3Local& localPoint) const;
	Mat3 getResponseMatrix(const Vec3Local& actionPoint, const Vec3Local& responsePoint) const;
	double getInertiaOfPointInDirectionLocal(const Vec3Local& localPoint, const Vec3Local& localDirection) const;
	double getInertiaOfPointInDirectionRelative(const Vec3Relative& relativePoint, const Vec3Relative& relativeDirection) const;
	inline Part* getMainPart() { return this->rigidBody.mainPart; }
	inline const Part* getMainPart() const { return this->rigidBody.mainPart; }

	void fullRefreshOfConnectedPhysicals();

	bool isValid() const;
};
