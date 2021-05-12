//When you have a word-size union which includes a rational number (e.g. mpq_class*),
//it can be an advantage to also have a word-size rational number. This way
//dynamic allocation can be avoided until necessary. Most operations must be checked
//to determine if they overflow/underflow, in which case they will return true.
//Fractions are simplified prior to division to avoid intermediate overflow.

#ifndef RAT64_T_H
#define RAT64_T_H

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include <numeric>

struct rat64_t{
    typedef int32_t SignedHalfWord;
    typedef uint32_t UnsignedHalfWord;
    typedef int64_t SignedWord;
    typedef uint64_t UnsignedWord;

    //TODO: This code calls abs() without considering abs(std::numeric_limits<SignedHalfWord>::min()) is UB
    //      probably should limit range so num == std::numeric_limits<SignedHalfWord>::min() is an "underflow"
    //      or figure out how to cast to UnsignedHalfWord

    static UnsignedHalfWord safeAbs(const SignedHalfWord& num){
        return std::abs(static_cast<SignedWord>(num));
    }

    static bool multWithOverflowCheck(UnsignedHalfWord a, UnsignedHalfWord b, UnsignedHalfWord& ans){
        UnsignedWord full = static_cast<UnsignedWord>(a) * static_cast<UnsignedWord>(b);
        ans = a*b;
        return full > std::numeric_limits<UnsignedHalfWord>::max();
    }

    static bool multWithOverflowCheck(SignedHalfWord a, SignedHalfWord b, SignedHalfWord& ans){
        SignedWord full = static_cast<SignedWord>(a) * static_cast<SignedWord>(b);
        ans = full;
        return full > std::numeric_limits<SignedHalfWord>::max() ||
               full < std::numeric_limits<SignedHalfWord>::min();
    }

    static bool multWithOverflowCheck(SignedHalfWord a, UnsignedHalfWord b, SignedHalfWord& ans){
        SignedWord full = static_cast<SignedWord>(a) * static_cast<SignedWord>(b);
        ans = full;
        return full > std::numeric_limits<SignedHalfWord>::max() ||
               full < std::numeric_limits<SignedHalfWord>::min();
    }

    static bool multWithOverflowCheck(UnsignedHalfWord a, SignedWord b, SignedHalfWord& ans){
        SignedWord full = static_cast<SignedWord>(a) * static_cast<SignedWord>(b);
        ans = full;
        return full > std::numeric_limits<SignedHalfWord>::max() ||
               full < std::numeric_limits<SignedHalfWord>::min();
    }

    SignedHalfWord num;
    UnsignedHalfWord den;

    rat64_t() : num(0), den(1) {}

    rat64_t(SignedHalfWord num) : num(num), den(1) {}

    rat64_t(SignedHalfWord num, UnsignedHalfWord den){
        assert(den!=0);
        const auto gcd = std::gcd(safeAbs(num), den);
        this->num = num / static_cast<int32_t>(gcd);
        this->den = den / gcd;
    }

    rat64_t(void* vpointer){
        memcpy(this, &vpointer, sizeof(rat64_t));
        assert(den!=0);
    }

    operator void*() const{
        assert(sizeof(void*) >= sizeof(rat64_t));
        void* vpointer;
        memcpy(&vpointer, this, sizeof(rat64_t));
        return vpointer;
    }

    operator double() const{
        return num / static_cast<double>(den);
    }

    static bool multiply(const rat64_t& lhs, const rat64_t& rhs, rat64_t& ans){
        const auto gcd1 = std::gcd(safeAbs(lhs.num), rhs.den);
        const auto gcd2 = std::gcd(safeAbs(rhs.num), lhs.den);

        const SignedHalfWord n1 = lhs.num/static_cast<SignedWord>(gcd1);
        const SignedHalfWord n2 = rhs.num/static_cast<SignedWord>(gcd2);
        const UnsignedHalfWord d1 = lhs.den/gcd2;
        const UnsignedHalfWord d2 = rhs.den/gcd1;

        return multWithOverflowCheck(d1, d2, ans.den) || multWithOverflowCheck(n1, n2, ans.num);
    }

    static bool divide(const rat64_t& lhs, const rat64_t& rhs, rat64_t& ans){
        const auto gcd1 = std::gcd(lhs.num, rhs.num);
        const auto gcd2 = std::gcd(rhs.den, lhs.den);

        const SignedHalfWord n1 = lhs.num/static_cast<SignedHalfWord>(gcd1);
        const UnsignedHalfWord n2 = rhs.den/static_cast<SignedHalfWord>(gcd2);
        const UnsignedHalfWord d1 = lhs.den/gcd2;
        const SignedHalfWord d2 = rhs.num/gcd1;

        return multWithOverflowCheck(n1, n2, ans.num) || multWithOverflowCheck(d1, d2, ans.den);
    }

    static bool multiply(const rat64_t& lhs, const UnsignedHalfWord& rhs, rat64_t& ans){
        const auto gcd = std::gcd(lhs.den, rhs);
        ans.den = lhs.den / gcd;
        return multWithOverflowCheck(lhs.num, rhs/gcd, ans.num);
    }

    inline static bool multiply(const UnsignedHalfWord& lhs, const rat64_t& rhs, rat64_t& ans){
        return multiply(rhs, lhs, ans);
    }

    static bool multiply(const rat64_t& lhs, const SignedHalfWord& rhs, rat64_t& ans){
        const auto gcd = std::gcd(lhs.den, safeAbs(rhs));
        ans.den = lhs.den / gcd;
        return multWithOverflowCheck(lhs.num, rhs/static_cast<SignedHalfWord>(gcd), ans.num);
    }

    inline static bool multiply(const SignedHalfWord& lhs, const rat64_t&rhs, rat64_t& ans){
        return multiply(rhs, lhs, ans);
    }

    static bool divide(const rat64_t& lhs, const UnsignedHalfWord& rhs, rat64_t& ans){
        const auto gcd = std::gcd(safeAbs(lhs.num), rhs);
        ans.num = lhs.num / gcd;
        return multWithOverflowCheck(lhs.den, rhs/gcd, ans.den);
    }

    static bool multiply(const rat64_t& lhs, const size_t& rhs, rat64_t& ans){
        const auto gcd = std::gcd(lhs.den, rhs);
        ans.den = lhs.den / gcd;
        const auto reduced_rhs = rhs/gcd;
        return reduced_rhs > std::numeric_limits<SignedHalfWord>::max() ||
               multWithOverflowCheck(lhs.num, static_cast<SignedHalfWord>(reduced_rhs), ans.num);
    }

    static bool multiply(const rat64_t& lhs, const long long& rhs, rat64_t& ans){
        const auto gcd = std::gcd(lhs.den, safeAbs(rhs));
        ans.den = lhs.den / gcd;
        const auto reduced_rhs = rhs/gcd;
        return reduced_rhs > std::numeric_limits<SignedHalfWord>::max() ||
               reduced_rhs < std::numeric_limits<SignedHalfWord>::min() ||
               multWithOverflowCheck(lhs.num, static_cast<SignedHalfWord>(reduced_rhs), ans.num);
    }

    static bool add(const SignedWord& ad, const SignedWord& bc, const UnsignedWord& bd, rat64_t& ans){
        // a/b + c/d = (a*d + b*c)/(b*d)

        if((ad >= 0) == (bc >= 0)){
            //The addition result may not fit in a signed word, but will fit in an unsigned word
            if(ad >= 0){
                UnsignedWord ad_bc =
                        static_cast<UnsignedWord>(ad)+static_cast<UnsignedWord>(bc);

                const auto gcd = std::gcd(ad_bc, bd);
                const auto den = bd / gcd;
                const auto unsigned_num = ad_bc / gcd;

                ans.num = unsigned_num;
                ans.den = den;

                return unsigned_num > std::numeric_limits<UnsignedHalfWord>::max() ||
                       den > std::numeric_limits<SignedHalfWord>::max();
            }else{
                UnsignedWord ad_bc =
                        static_cast<UnsignedWord>(-ad)+static_cast<UnsignedWord>(-bc);

                const UnsignedWord gcd = std::gcd(ad_bc, bd);
                const UnsignedWord den = bd / gcd;
                const UnsignedWord unsigned_num = ad_bc / gcd;

                ans.num = -unsigned_num;
                ans.den = den;

                return unsigned_num > std::numeric_limits<UnsignedHalfWord>::max() ||
                       den > -(SignedWord)std::numeric_limits<SignedHalfWord>::min();
            }
        }else{
            //The addition result will fit in a signed word
            SignedWord ad_bc = ad + bc;

            const UnsignedWord gcd = std::gcd(safeAbs(ad_bc), bd);

            const SignedWord num = ad_bc / static_cast<SignedWord>(gcd);
            const UnsignedWord den = bd / gcd;

            ans.num = num;
            ans.den = den;

            return num > std::numeric_limits<SignedHalfWord>::max() ||
                   num < std::numeric_limits<SignedHalfWord>::min() ||
                   den > std::numeric_limits<UnsignedHalfWord>::max();
        }
    }

    static bool add(const rat64_t& lhs, const rat64_t& rhs, rat64_t& ans){
        // a/b + c/d = (a*d + b*c)/(b*d)

        const SignedWord ad = static_cast<SignedWord>(rhs.num)*static_cast<SignedWord>(lhs.den);
        const SignedWord bc = static_cast<SignedWord>(lhs.num)*static_cast<SignedWord>(rhs.den);
        const UnsignedWord bd = static_cast<UnsignedWord>(lhs.den)*static_cast<UnsignedWord>(rhs.den);

        return add(ad, bc, bd, ans);
    }

    static bool subtract(const rat64_t& lhs, const rat64_t& rhs, rat64_t& ans){
        // a/b - c/d = (a*d - b*c)/(b*d)

        UnsignedWord bd = static_cast<UnsignedWord>(lhs.den) * static_cast<UnsignedWord>(rhs.den);
        SignedWord ad = static_cast<UnsignedWord>(rhs.num)*static_cast<UnsignedWord>(lhs.den);
        SignedWord bc = -static_cast<UnsignedWord>(lhs.num)*static_cast<UnsignedWord>(rhs.den);

        //Make sure to convert to larger type before negating, because
        // -std::numeric_limits<SignedWord>::min() is UB

        return add(ad, bc, bd, ans);
    }

    static bool add(const rat64_t& lhs, const SignedHalfWord& rhs, rat64_t& ans){
        //The addition result will fit in a signed word
        const SignedWord ad_bc = static_cast<SignedWord>(rhs)*static_cast<SignedWord>(lhs.den) + lhs.num;
        const UnsignedWord gcd = std::gcd(safeAbs(ad_bc), lhs.den);
        const SignedWord num = ad_bc / static_cast<SignedWord>(gcd);
        const UnsignedWord den = lhs.den / gcd;

        ans.num = num;
        ans.den = den;

        return num > std::numeric_limits<SignedHalfWord>::max() ||
               num < std::numeric_limits<SignedHalfWord>::min() ||
               den > std::numeric_limits<UnsignedHalfWord>::max();
    }

    static bool power(const rat64_t& lhs, const uint8_t& rhs, rat64_t& ans){
        SignedWord num = 1;
        UnsignedWord den = 1;

        for(int i = 0; i < rhs; i++){
            num *= lhs.num;
            den *= lhs.den;
            if(num > std::numeric_limits<SignedHalfWord>::max() ||
               num < std::numeric_limits<SignedHalfWord>::min() ||
               den > std::numeric_limits<UnsignedHalfWord>::max())
                return true;
        }

        ans.num = num;
        ans.den = den;
        return false;
    }

    std::string toStr() const{
        return std::to_string(num) + '/' + std::to_string(den);
    }

    friend std::ostream& operator<<(std::ostream& out, const rat64_t& rat){
        out << std::to_string(rat.num) + '/' + std::to_string(rat.den);
        return out;
    }

    bool operator==(const rat64_t& rhs) const{
        assert(std::gcd(safeAbs(num), den) == 1);
        assert(std::gcd(safeAbs(rhs.num), rhs.den) == 1);
        return num == rhs.num && den == rhs.den;
    }

    bool operator!=(const rat64_t& rhs) const{
        assert(std::gcd(safeAbs(num), den) == 1);
        assert(std::gcd(safeAbs(rhs.num), rhs.den) == 1);
        return num != rhs.num || den != rhs.den;
    }

    bool operator<(const rat64_t& rhs) const{
        assert(std::gcd(safeAbs(num), den) == 1);
        assert(std::gcd(safeAbs(rhs.num), rhs.den) == 1);
        return
            static_cast<SignedWord>(num)*static_cast<SignedWord>(rhs.den)
            <
            static_cast<SignedWord>(rhs.num)*static_cast<SignedWord>(den);
    }

    bool operator<(int32_t rhs) const{
        return num < static_cast<SignedWord>(rhs) * static_cast<SignedWord>(den);
    }

    bool operator<=(const rat64_t& rhs) const{
        assert(std::gcd(safeAbs(num), den) == 1);
        assert(std::gcd(safeAbs(rhs.num), rhs.den) == 1);
        return
            static_cast<SignedWord>(num)*static_cast<SignedWord>(rhs.den)
            <=
            static_cast<SignedWord>(rhs.num)*static_cast<SignedWord>(den);
    }

    bool operator>(const rat64_t& rhs) const{
        assert(std::gcd(safeAbs(num), den) == 1);
        assert(std::gcd(safeAbs(rhs.num), rhs.den) == 1);
        return
            static_cast<SignedWord>(num)*static_cast<SignedWord>(rhs.den)
            >
            static_cast<SignedWord>(rhs.num)*static_cast<SignedWord>(den);
    }

    bool operator>=(const rat64_t& rhs) const{
        assert(std::gcd(safeAbs(num), den) == 1);
        assert(std::gcd(safeAbs(rhs.num), rhs.den) == 1);
        return
            static_cast<SignedWord>(num)*static_cast<SignedWord>(rhs.den)
            >=
            static_cast<SignedWord>(rhs.num)*static_cast<SignedWord>(den);
    }

    bool greaterThanPi() const{
        //165707065/52746197 is the closest rat64_t to pi. It is greater by ~1.64084e-16.
        //80143857/25510582 is the closest rat64_t less than pi. It is less by ~5.79087e-16
        //Compare with the double approximation π-4*atan(1) with an error of ~3.4641e-07
        return 52746197L*static_cast<SignedWord>(num) >= 165707065L*static_cast<SignedWord>(den);
    }
};

#endif // RAT64_T_H
