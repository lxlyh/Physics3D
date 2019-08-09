#pragma once

#include "position.h"

struct Bounds {
	Position min;
	Position max;

	inline Bounds() = default;
	inline Bounds(const Position& min, const Position& max) : min(min), max(max) {}

	inline Vec3Fix getDiagonal() const {
		return max - min;
	}

	inline bool contains(const Position& p) const {
		return p >= min && p <= max;
	}

	inline bool contains(const Bounds& other) const {
		return other.min >= this->min && other.max <= this->max;
	}

	inline Position getCenter() const {
		return avg(min, max);
	}

	inline Bounds expanded(Fix<32> amount) const {
		return Bounds(min - Vec3Fix(amount, amount, amount), max + Vec3Fix(amount, amount, amount));
	}

	inline Bounds expanded(Vec3Fix amount) const {
		return Bounds(min - amount, max + amount);
	}

	inline Fix<32> getWidth() const { return max.x - min.x; }
	inline Fix<32> getHeight() const { return max.y - min.y; }
	inline Fix<32> getDepth() const { return max.z - min.z; }
};

inline bool intersects(const Bounds& first, const Bounds& second) {
	return first.max >= second.min && first.min <= second.max;
}
inline Bounds unionOfBounds(const Bounds& first, const Bounds& second) {
	return Bounds(min(first.min, second.min), max(first.max, second.max));
}
