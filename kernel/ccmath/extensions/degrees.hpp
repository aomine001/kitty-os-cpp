/*
 * Copyright (c) 2024-Present Ian Pike
 * Copyright (c) 2024-Present ccmath contributors
 *
 * This library is provided under the MIT License.
 * See LICENSE for more information.
 */

#pragma once

#include "../numbers.hpp"
#include <type_traits>

namespace ccm::ext
{
	/**
	 * @brief Converts an angle in radians to degrees.
	 * @tparam T Floating-point type.
	 * @param radians Angle in radians.
	 * @return Angle in degrees.
	 */
	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr T degrees(T radians) noexcept
    {
        return (static_cast<T>(180) * radians) / ccm::numbers::pi_v<T>;
    }
} // namespace ccm::ext
