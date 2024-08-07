/*
 * Copyright (c) 2024-Present Ian Pike
 * Copyright (c) 2024-Present ccmath contributors
 *
 * This library is provided under the MIT License.
 * See LICENSE for more information.
 */

#pragma once

#include <cfenv>

#include "../predef/unlikely.hpp"
#include "../support/bits.hpp"
#include "../support/fenv/fenv_support.hpp"
#include "../support/fenv/rounding_mode.hpp"
#include "../support/fp/fp_bits.hpp"
#include "../types/dyadic_float.hpp"

namespace ccm::helpers
{
	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T internal_ldexp(T x, int exp)
	{
		support::fp::FPBits<T> bits(x);
		if (CCM_UNLIKELY((exp == 0) || bits.is_zero() || bits.is_inf_or_nan())) { return x; }

		constexpr int EXP_LIMIT = support::fp::FPBits<T>::MAX_BIASED_EXPONENT + support::fp::FPBits<T>::fraction_length + 1;
		if (CCM_UNLIKELY(exp > EXP_LIMIT))
		{
			int const rounding_mode = support::fenv::get_rounding_mode();
			types::Sign sign		= bits.sign();

			if ((sign == types::Sign::POS && rounding_mode == FE_DOWNWARD) || (sign == types::Sign::NEG && rounding_mode == FE_UPWARD) ||
				(rounding_mode == FE_TOWARDZERO))
			{
				return support::fp::FPBits<T>::max_normal(sign).get_val();
			}

			// These functions do nothing at compile time, but at runtime will set errno and raise exceptions if required.
			support::fenv::set_errno_if_required(ERANGE);
			support::fenv::raise_except_if_required(FE_OVERFLOW);

			return support::fp::FPBits<T>::inf(sign).get_val();
		}

		if (CCM_UNLIKELY(exp < -EXP_LIMIT))
		{
			int const rounding_mode = support::fenv::get_rounding_mode();
			types::Sign sign		= bits.sign();

			if ((sign == types::Sign::POS && rounding_mode == FE_UPWARD) || (sign == types::Sign::NEG && rounding_mode == FE_DOWNWARD))
			{
				return support::fp::FPBits<T>::min_subnormal(sign).get_val();
			}

			// These functions do nothing at compile time, but at runtime will set errno and raise exceptions if required.
			support::fenv::set_errno_if_required(ERANGE);
			support::fenv::raise_except_if_required(FE_UNDERFLOW);

			return support::fp::FPBits<T>::zero(sign).get_val();
		}

		// For all other values, NormalFloat to T conversion handles it the right way.
		types::DyadicFloat<support::fp::FPBits<T>::storage_length> normal(bits.get_val());
		normal.exponent += exp;
		return static_cast<T>(normal);
	}
} // namespace ccm::helpers