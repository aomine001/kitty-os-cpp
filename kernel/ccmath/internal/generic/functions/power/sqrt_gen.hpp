/*
 * Copyright (c) 2024-Present Ian Pike
 * Copyright (c) 2024-Present ccmath contributors
 *
 * This library is provided under the MIT License.
 * See LICENSE for more information.
 */

#pragma once

#include "../../../config/type_support.hpp"
#include "../../../predef/unlikely.hpp"
#include "../../../runtime/simd/simd_vectorize.hpp"
#include "../../../support/always_false.hpp"
#include "../../../support/bits.hpp"
#include "../../../support/fenv/rounding_mode.hpp"
#include "../../../support/fp/fp_bits.hpp"
#include "../../../support/is_constant_evaluated.hpp"

#include <type_traits>

namespace ccm::gen
{
	namespace internal
	{

		template <typename T>
		struct Is80BitLongDouble
		{
			static constexpr bool value = false;
		};

		template <typename T>
		constexpr void normalize(int & exponent, typename support::fp::FPBits<T>::storage_type & mantissa)
		{
			const int shift = support::countl_zero(mantissa) - (8 * static_cast<int>(sizeof(mantissa)) - 1 - support::fp::FPBits<T>::fraction_length);
			exponent -= shift;
			mantissa <<= shift;
		}

#if defined(CCM_TYPES_LONG_DOUBLE_IS_FLOAT80)
		template <>
		struct Is80BitLongDouble<long double>
		{
			static constexpr bool value = true;
		};
#endif

#if defined(CCM_TYPES_LONG_DOUBLE_IS_FLOAT64)
		template <>
		constexpr void normalize<long double>(int & exponent, std::uint64_t & mantissa)
		{
			normalize<double>(exponent, mantissa);
		}
#elif defined(CCM_TYPES_LONG_DOUBLE_IS_FLOAT80)
		template <>
		constexpr void normalize<long double>(int & exponent, types::uint128_t & mantissa)
		{
			const auto shift = static_cast<unsigned int>(static_cast<unsigned long>(support::countl_zero(static_cast<std::uint64_t>(mantissa))) -
														 (8 * sizeof(std::uint64_t) - 1 - support::fp::FPBits<long double>::fraction_length));
			exponent -= shift; // NOLINT(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
			mantissa <<= shift;
		}
#endif

		template <typename T>
		constexpr bool Is80BitLongDouble_v = Is80BitLongDouble<T>::value;

		namespace impl
		{
			namespace bit80
			{
				// This has to be defined for sqrt_impl to work as it still needs to see that this function exists
				constexpr long double sqrt_calc_80bits(long double x);

#if defined(CCM_TYPES_LONG_DOUBLE_IS_FLOAT80)
				constexpr long double sqrt_calc_80bits(long double x)
				{
					using Bits				   = support::fp::FPBits<long double>;
					using storage_type		   = typename Bits::storage_type;
					constexpr storage_type one = static_cast<storage_type>(1) << Bits::fraction_length;
					constexpr auto nan_type	   = Bits::quiet_nan().get_val();

					const Bits bits(x);

					if (bits == Bits::inf(types::Sign::POS) || bits.is_zero() || bits.is_nan()) { return x; }
					if (bits.is_neg()) { return nan_type; }

					int x_exp			= bits.get_explicit_exponent();
					storage_type x_mant = bits.get_mantissa();

					// If we have denormal values, normalize it.
					if (bits.get_implicit_bit()) { x_mant |= one; }
					else if (bits.is_subnormal()) { normalize<long double>(x_exp, x_mant); }

					// Ensure that the exponent is even.
					if ((x_exp & 1) != 0)
					{
						--x_exp;
						x_mant <<= 1;
					}

					storage_type y = one;
					storage_type r = x_mant - one;

					if (ccm::support::is_constant_evaluated())
					{
						for (storage_type current_bit = one >> 1; current_bit != 0U; current_bit >>= 1)
						{
							r <<= 1;
							if (const storage_type tmp = (y << 1) + current_bit; r >= tmp)
							{
								r -= tmp;
								y += current_bit;
							}
						}
					}
					else // If we are not in a constant evaluated context, we can vectorize the loop.
					{
						CCM_SIMD_VECTORIZE for (storage_type current_bit = one >> 1; current_bit != 0U; current_bit >>= 1)
						{
							r <<= 1;
							if (const storage_type tmp = (y << 1) + current_bit; r >= tmp)
							{
								r -= tmp;
								y += current_bit;
							}
						}
					}

					// We perform one more iteration to ensure that the result is correctly rounded.
					const auto lsb = static_cast<bool>(y & 1);
					bool round_bit = false;
					r <<= 2;
					if (const storage_type tmp = (y << 2) + 1; r >= tmp)
					{
						r -= tmp;
						round_bit = true;
					}

					// Append the exponent field.
					x_exp = ((x_exp >> 1) + Bits::exponent_bias);
					y |= (static_cast<storage_type>(x_exp) << (Bits::fraction_length + 1));

					switch (support::fenv::get_rounding_mode())
					{
					case FE_TONEAREST:
						// Round to nearest, ties to even
						if (round_bit && (lsb || (r != 0))) { ++y; }
						break;
					case FE_UPWARD:
						if (round_bit || (r != 0)) { ++y; }
						break;
					default: break;
					}

					// Extract output
					support::fp::FPBits<long double> out(0.0L);
					out.set_biased_exponent(static_cast<storage_type>(x_exp));
					out.set_implicit_bit(true);
					out.set_mantissa(y & (one - 1));

					return out.get_val();
				}
#endif
			} // namespace bit80

			template <typename T>
			static constexpr std::enable_if_t<std::is_floating_point_v<T>, T> sqrt_calc_bits(const support::fp::FPBits<T> & bits)
			{
				using FPBits_t			   = support::fp::FPBits<T>;
				using storage_type		   = typename FPBits_t::storage_type;
				constexpr storage_type one = storage_type(1) << FPBits_t::fraction_length;

				int x_exp			= bits.get_exponent();
				storage_type x_mant = bits.get_mantissa();

				// If we have denormal values, normalize it.
				if (bits.is_subnormal())
				{
					++x_exp; // ensure that x_exp is the correct exponent of one bit.
					internal::normalize<T>(x_exp, x_mant);
				}
				else { x_mant |= one; }

				// Ensure that the exponent is even.
				if (x_exp & 1)
				{
					--x_exp;
					x_mant <<= 1;
				}

				storage_type y = one;
				storage_type r = x_mant - one;

				if (ccm::support::is_constant_evaluated())
				{
					for (storage_type current_bit = one >> 1; current_bit; current_bit >>= 1)
					{
						r <<= 1;
						storage_type const tmp = (y << 1) + current_bit; // 2*y(n - 1) + 2^(-n-1)
						if (r >= tmp)
						{
							r -= tmp;
							y += current_bit;
						}
					}
				}
				else // If we are not in a constant evaluated context, we can vectorize the loop.
				{
					CCM_SIMD_VECTORIZE for (storage_type current_bit = one >> 1; current_bit; current_bit >>= 1)
					{
						r <<= 1;
						storage_type const tmp = (y << 1) + current_bit; // 2*y(n - 1) + 2^(-n-1)
						if (r >= tmp)
						{
							r -= tmp;
							y += current_bit;
						}
					}
				}

				// We perform one more iteration to ensure that the result is correctly rounded.
				auto lsb	   = static_cast<bool>(y & 1); // Least significant bit
				bool round_bit = false;

				r <<= 2;

				if (storage_type const tmp = (y << 2) + 1; r >= tmp)
				{
					r -= tmp;
					round_bit = true;
				}

				// Remove the hidden bit and append the exponent field.
				x_exp = ((x_exp >> 1) + FPBits_t::exponent_bias);

				y = (y - one) | (static_cast<storage_type>(x_exp) << FPBits_t::fraction_length);

				switch (support::fenv::get_rounding_mode())
				{
				case FE_TONEAREST:
					// Round to nearest, ties to even
					if (round_bit && (lsb || (r != 0))) { ++y; }
					break;
				case FE_UPWARD:
					if (round_bit || (r != 0)) { ++y; }
					break;
				default: break;
				}

				return support::bit_cast<T>(y);
			}

			// This calculates the square root of any IEEE-754 floating point number using the shift and add algorithm.
			// The function accounts for all rounding modes and special cases.
			template <typename T>
			static constexpr std::enable_if_t<std::is_floating_point_v<T>, T> sqrt_impl(T x) // NOLINT
			{
				if constexpr (Is80BitLongDouble_v<T>) { return bit80::sqrt_calc_80bits(x); }
				else
				{
					// IEEE floating points formats.
					using FPBits_t		   = support::fp::FPBits<T>;
					constexpr auto flt_nan = FPBits_t::quiet_nan().get_val();

					const FPBits_t bits(x);

					// bits.uin

					// lut_double_sqrt_first_256
					//  Handle special cases where the bits are +Inf, ±0, or ±NaN
					if (CCM_UNLIKELY(bits == FPBits_t::inf(types::Sign::POS) || bits.is_zero() || bits.is_nan())) { return x; }

					// Handle special cases where the bits are -Inf or -num
					if (CCM_UNLIKELY(bits.is_neg())) { return -flt_nan; }

					// If we didn't encounter any special cases, we can calculate the square root normally.
					return sqrt_calc_bits(bits);
				}
			}
		} // namespace impl
	} // namespace internal

	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
	constexpr T sqrt_gen(T num)
	{
		return internal::impl::sqrt_impl(num);
	}

} // namespace ccm::gen
