// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nltime.h
 */

#ifndef NLTIME_H_
#define NLTIME_H_

#include "nl_config.h"
#include "plib/pstate.h"
#include "plib/ptypes.h"

#include <cstdint>

// ----------------------------------------------------------------------------------------
// netlist_time
// ----------------------------------------------------------------------------------------

namespace netlist
{

	template <typename TYPE, TYPE RES>
	struct ptime final
	{
	public:

		using internal_type = TYPE;
		using mult_type = TYPE;

		constexpr ptime() noexcept : m_time(0) {}

		~ptime() noexcept = default;

		constexpr ptime(const ptime &rhs) noexcept = default;
		constexpr ptime(ptime &&rhs) noexcept = default;
		constexpr explicit ptime(const internal_type &time) noexcept : m_time(time) {}
		constexpr explicit ptime(internal_type &&time) noexcept : m_time(time) {}
		C14CONSTEXPR ptime &operator=(const ptime &rhs) noexcept = default;
		C14CONSTEXPR ptime &operator=(ptime &&rhs) noexcept  = default;

		constexpr explicit ptime(const double t) = delete;
		//: m_time((internal_type) ( t * (double) resolution)) { }
		constexpr explicit ptime(const internal_type nom, const internal_type den) noexcept
		: m_time(nom * (RES / den)) { }

		C14CONSTEXPR ptime &operator+=(const ptime rhs) noexcept { m_time += rhs.m_time; return *this; }
		C14CONSTEXPR ptime &operator-=(const ptime rhs) noexcept { m_time -= rhs.m_time; return *this; }
		C14CONSTEXPR ptime &operator*=(const mult_type factor) noexcept { m_time *= static_cast<internal_type>(factor); return *this; }

		friend C14CONSTEXPR ptime operator-(ptime lhs, const ptime rhs) noexcept
		{
			return lhs -= rhs;
		}

		friend C14CONSTEXPR ptime operator+(ptime lhs, const ptime rhs) noexcept
		{
			return lhs += rhs;
		}

		friend C14CONSTEXPR ptime operator*(ptime lhs, const mult_type &factor) noexcept
		{
			return lhs *= factor;
		}

		friend constexpr mult_type operator/(const ptime lhs, const ptime rhs) noexcept
		{
			return static_cast<mult_type>(lhs.m_time / rhs.m_time);
		}

		friend constexpr bool operator<(const ptime lhs, const ptime rhs) noexcept
		{
			return (lhs.m_time < rhs.m_time);
		}

		friend constexpr bool operator>(const ptime lhs, const ptime rhs) noexcept
		{
			return (rhs < lhs);
		}

		friend constexpr bool operator<=(const ptime lhs, const ptime rhs) noexcept
		{
			return !(lhs > rhs);
		}

		friend constexpr bool operator>=(const ptime lhs, const ptime rhs) noexcept
		{
			return !(lhs < rhs);
		}

		friend constexpr bool operator==(const ptime lhs, const ptime rhs) noexcept
		{
			return lhs.m_time == rhs.m_time;
		}

		friend constexpr bool operator!=(const ptime lhs, const ptime rhs) noexcept
		{
			return !(lhs == rhs);
		}

		constexpr internal_type as_raw() const noexcept { return m_time; }
		constexpr double as_double() const noexcept
		{
			return static_cast<double>(m_time) * inv_res;
		}

		// for save states ....
		C14CONSTEXPR internal_type *get_internaltype_ptr() noexcept { return &m_time; }

		static constexpr ptime from_nsec(const internal_type ns) noexcept { return ptime(ns, UINT64_C(1000000000)); }
		static constexpr ptime from_usec(const internal_type us) noexcept { return ptime(us, UINT64_C(   1000000)); }
		static constexpr ptime from_msec(const internal_type ms) noexcept { return ptime(ms, UINT64_C(      1000)); }
		static constexpr ptime from_sec(const internal_type s) noexcept   { return ptime(s,  UINT64_C(         1)); }
		static constexpr ptime from_hz(const internal_type hz) noexcept { return ptime(1 , hz); }
		static constexpr ptime from_raw(const internal_type raw) noexcept { return ptime(raw); }
		static constexpr ptime from_double(const double t) noexcept { return ptime(static_cast<internal_type>( t * static_cast<double>(RES)), RES); }

		static constexpr ptime zero() noexcept { return ptime(0, RES); }
		static constexpr ptime quantum() noexcept { return ptime(1, RES); }
		static constexpr ptime never() noexcept { return ptime(plib::numeric_limits<internal_type>::max(), RES); }
		static constexpr internal_type resolution() noexcept { return RES; }

		constexpr internal_type in_nsec() const noexcept { return m_time / (RES / UINT64_C(1000000000)); }
		constexpr internal_type in_usec() const noexcept { return m_time / (RES / UINT64_C(   1000000)); }
		constexpr internal_type in_msec() const noexcept { return m_time / (RES / UINT64_C(      1000)); }
		constexpr internal_type in_sec()  const noexcept { return m_time / (RES / UINT64_C(         1)); }

	private:
		static constexpr const double inv_res = 1.0 / static_cast<double>(RES);
		internal_type m_time;
	};

#if (PHAS_INT128)
	using netlist_time = ptime<UINT128, NETLIST_INTERNAL_RES>;
#else
	using netlist_time = ptime<std::int64_t, NETLIST_INTERNAL_RES>;
	static_assert(noexcept(netlist_time::from_nsec(1)) == true, "Not evaluated as constexpr");
#endif

	//============================================================
	//  MACROS
	//============================================================

	template <typename T> inline constexpr netlist_time NLTIME_FROM_NS(T &&t) noexcept { return netlist_time::from_nsec(t); }
	template <typename T> inline constexpr netlist_time NLTIME_FROM_US(T &&t) noexcept { return netlist_time::from_usec(t); }
	template <typename T> inline constexpr netlist_time NLTIME_FROM_MS(T &&t) noexcept { return netlist_time::from_msec(t); }

} // namespace netlist

namespace plib {

	template<> inline void state_manager_t::save_item(const void *owner, netlist::netlist_time &nlt, const pstring &stname)
	{
		save_state_ptr(owner, stname, datatype_t(sizeof(netlist::netlist_time::internal_type), true, false), 1, nlt.get_internaltype_ptr());
	}
} // namespace plib

#endif /* NLTIME_H_ */
