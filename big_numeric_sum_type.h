#ifndef BIG_NUMERIC_SUM_TYPE_H
#define BIG_NUMERIC_SUM_TYPE_H

//This creates a truly terrible matrix of features
//It is certainly more convenient to use mpq_class for everything
//and only reduce when canonicalizing.
//
//The benchmarks in release show multiplication is ~2.5x faster on the sum type in word rational range,
//and ~12x faster on the sum type in word integer range
//
//There is also a 10x difference for multiplying a big integer by a little integer using a sum type
//as opposed to using rationals
//
//There is a significant performance payoff. I'm still not sure it justifies the complexity!

#include "rat64_t.h"
#include <gmpxx.h>

enum Type{
    GmpInt,
    GmpRat,
    WordInt,
    WordRat,
};

constexpr inline uint16_t typePair(Type a, Type b) noexcept{
    return a + (b << 2);
}

struct NumType{
    void* data;
    Type type;

    inline int64_t asWordInt() const noexcept {
        assert(type == WordInt);
        return reinterpret_cast<int64_t>(data);
    }
    inline rat64_t asWordRat() const noexcept {
        assert(type == WordRat);
        return static_cast<rat64_t>(data);
    }
    inline mpz_class& asBigInt() const noexcept {
        assert(type == GmpInt);
        return *reinterpret_cast<mpz_class*>(data);
    }
    inline mpq_class& asBigRat() const noexcept {
        assert(type == GmpRat);
        return *reinterpret_cast<mpq_class*>(data);
    }

    void bigIntReduce() noexcept{
        auto z = asBigInt();
        if(mpz_fits_slong_p(z.get_mpz_t())){
            int64_t next = z.get_si();
            delete reinterpret_cast<mpz_class*>(data);
            data = reinterpret_cast<void*>(next);
            type = WordInt;
        }
    }

    void bigRatReduce(){
        auto r = asBigRat();
        r.canonicalize();

        if(r.get_den() == 1){
            if(mpz_fits_slong_p(r.get_num_mpz_t())){
                int64_t next = r.get_num().get_si();
                delete reinterpret_cast<mpq_class*>(data);
                data = reinterpret_cast<void*>(next);
                type = WordInt;
            }else{
                mpz_class* next = new mpz_class(r.get_num());
                delete reinterpret_cast<mpq_class*>(data);
                data = next;
                type = GmpInt;
            }
        }else if(r.get_den() <= std::numeric_limits<uint32_t>::max() && mpz_fits_slong_p(r.get_num_mpz_t())){
            rat64_t next(r.get_num().get_si(), r.get_den().get_ui());
            delete reinterpret_cast<mpq_class*>(data);
            data = next;
            type = WordRat;
        }
    }

    void wordRatReduce(){
        rat64_t r = asWordRat();
        r.canonicalize();

        if(r.den==1){
            data = reinterpret_cast<void*>(r.num);
            type = WordInt;
        }else{
            data = r;
        }
    }

    void reduce(){
        switch (type) {
            case WordRat: wordRatReduce(); break;
            case GmpInt: bigIntReduce(); break;
            case GmpRat: bigRatReduce(); break;
            case WordInt: break;
        }
    }

    void wordIntClamp(){
        auto z = asWordInt();
        if(z > std::numeric_limits<int32_t>::max() || z < std::numeric_limits<int32_t>::min()){
            mpz_class* next = new mpz_class(std::to_string(z)); //This isn't great
            data = next;
            type = GmpInt;
        }
    }

    NumType(int32_t val) : data(reinterpret_cast<void*>(val)), type(WordInt) {}
    NumType(rat64_t::SignedHalfWord num, rat64_t::UnsignedHalfWord den) : data(rat64_t(num, den)), type(WordRat){}
    NumType(const mpz_class& val) : data(new mpz_class(val)), type(GmpInt) {}
    NumType(const mpq_class& val) : data(new mpq_class(val)), type(GmpRat) {}
    ~NumType(){
        if(type == GmpInt) delete reinterpret_cast<mpz_class*>(data);
        else if(type == GmpRat) delete reinterpret_cast<mpq_class*>(data);
    }

    std::string toString() const{
        switch (type) {
            case WordInt: return std::to_string(asWordInt());
            case WordRat: return asWordRat().toStr();
            case GmpInt: return asBigInt().get_str();
            case GmpRat: return asBigRat().get_str();
            default: assert(false);
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const NumType& num){
        out << num.toString();
        return out;
    }

    template<bool reduce = true>
    NumType operator-() const noexcept{
        switch (type) {
            case WordInt:{
                int64_t z = asWordInt();
                if(z == std::numeric_limits<int32_t>::min()){
                    mpz_class next(std::numeric_limits<int32_t>::min());
                    next = -next;
                    return NumType(next);
                }else{
                    return NumType(-z);
                }
            }
            case WordRat:{
                rat64_t r = asWordRat();
                if(r.num == std::numeric_limits<int32_t>::min()){
                    mpq_class next(r.num, r.den);
                    next = -next;
                    return NumType(next);
                }else{
                    return NumType(-r.num, r.den);
                }
            }
            case GmpInt:{
                mpz_class temp = asBigInt();
                temp = -temp;
                if(reduce && temp == std::numeric_limits<int32_t>::min())
                    return NumType(std::numeric_limits<int32_t>::min());
                else
                    return NumType(temp);
            }
            case GmpRat: return NumType(-asBigRat());
        }
    }

    bool operator==(const NumType& other) const noexcept{
        if(type != other.type) return false;
        else if(type >= WordInt) return data == other.data;
        else if(type == GmpInt) return asBigInt() == other.asBigInt();
        else return asBigRat() == other.asBigRat();
    }

    bool operator!=(const NumType& other) const noexcept{
        if(type != other.type) return true;
        else if(type >= WordInt) return data != other.data;
        else if(type == GmpInt) return asBigInt() != other.asBigInt();
        else return asBigRat() != other.asBigRat();
    }

    bool operator<(const NumType& other) const{
        switch (typePair(type, other.type)) {
            case typePair(WordInt, WordInt): return asWordInt() < other.asWordInt();
            case typePair(WordInt, WordRat): return asWordInt() < other.asWordRat();
            case typePair(WordInt, GmpInt): return (int32_t)asWordInt() < other.asBigInt();
            case typePair(WordInt, GmpRat): return (int32_t)asWordInt() < other.asBigRat();
            case typePair(WordRat, WordInt): return asWordRat() < (int32_t)other.asWordInt();
            case typePair(WordRat, WordRat): return asWordRat() < other.asWordRat();
            case typePair(WordRat, GmpInt): return asWordRat().num < other.asBigInt()*asWordRat().den;
            case typePair(WordRat, GmpRat): return 1 < other.asBigRat()*asWordRat().den/asWordRat().num;
            case typePair(GmpInt, WordInt): return asBigInt() < (int32_t)other.asWordInt();
            case typePair(GmpInt, WordRat): return asBigInt()*other.asWordRat().den < other.asWordRat().num;
            case typePair(GmpInt, GmpInt): return asBigInt() < other.asBigInt();
            case typePair(GmpInt, GmpRat): return asBigInt() < other.asBigRat();
            case typePair(GmpRat, WordInt): return asBigRat() < (int32_t)other.asWordInt();
            case typePair(GmpRat, WordRat): return asBigRat()*other.asWordRat().den/other.asWordRat().num < 1;
            case typePair(GmpRat, GmpInt): return asBigRat() < other.asBigInt();
            case typePair(GmpRat, GmpRat): return asBigRat() < other.asBigRat();
        }

        assert(false);
    }

    template<bool reduce = true>
    void operator*=(const NumType& other){
        switch(typePair(type, other.type)){
            case typePair(WordInt, WordInt):
                data = reinterpret_cast<void*>(asWordInt() * other.asWordInt());
                wordIntClamp();
                break;
            case typePair(WordInt, WordRat):{
                rat64_t ans;
                int32_t lhs = asWordInt();
                rat64_t rhs = other.asWordRat();
                if(rat64_t::multiply(lhs, rhs, ans)){
                    mpq_class* result = new mpq_class(rhs.num, rhs.den);
                    result->operator*=(lhs);
                    data = result;
                    type = GmpRat;
                    if(reduce) bigRatReduce();
                }else if(ans.den == 1){
                    data = reinterpret_cast<void*>(ans.num);
                }else{
                    data = ans;
                    type = WordRat;
                }
                break;
            }
            case typePair(WordInt, GmpInt):{
                if(data == 0) break;
                data = new mpz_class(other.asBigInt()*(int32_t)asWordInt());
                type = GmpInt;
                break;
            }
            case typePair(WordInt, GmpRat):{
                if(data == 0) break;
                data = new mpq_class(other.asBigRat()*(int32_t)asWordInt());
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(WordRat, WordInt):{
                rat64_t ans;
                rat64_t lhs = asWordRat();
                int32_t rhs = other.asWordInt();
                if(rat64_t::multiply(lhs, rhs, ans)){
                    mpq_class* result = new mpq_class(lhs.num, lhs.den);
                    result->operator*=(rhs);
                    data = result;
                    type = GmpRat;
                    if(reduce) bigRatReduce();
                }else if(ans.den == 1){
                    data = reinterpret_cast<void*>(ans.num);
                    type = WordInt;
                }else{
                    data = ans;
                }
                break;
            }
            case typePair(WordRat, WordRat):{
                rat64_t ans;
                rat64_t lhs = asWordRat();
                rat64_t rhs = other.asWordRat();
                if(rat64_t::multiply(lhs, rhs, ans)){
                    mpq_class* result = new mpq_class(lhs.num, lhs.den);
                    result->operator*=(rhs.num);
                    result->operator/=(rhs.den);
                    data = result;
                    type = GmpRat;
                    if(reduce) bigRatReduce();
                }else if(ans.den == 1){
                    data = reinterpret_cast<void*>(ans.num);
                    type = WordInt;
                }else{
                    data = ans;
                }
                break;
            }
            case typePair(WordRat, GmpInt):{
                mpq_class* next = new mpq_class(other.asBigInt());
                next->operator*=(asWordRat().num);
                next->operator/=(asWordRat().den);
                data = next;
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(WordRat, GmpRat):{
                mpq_class* next = new mpq_class(other.asBigRat());
                next->operator*=(asWordRat().num);
                next->operator/=(asWordRat().den);
                data = next;
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(GmpInt, WordInt):
                if(other.data == 0){
                    delete reinterpret_cast<mpz_class*>(data);
                    data = 0;
                    type = WordInt;
                }else{
                    asBigInt() *= (int32_t)other.asWordInt();
                }
                break;
            case typePair(GmpInt, WordRat):{
                mpq_class* next = new mpq_class(asBigInt());
                delete reinterpret_cast<mpz_class*>(data);
                next->operator*=(other.asWordRat().num);
                next->operator*=(other.asWordRat().den);
                data = next;
                type = WordRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(GmpInt, GmpInt):
                asBigInt() *= other.asBigInt();
                break;
            case typePair(GmpInt, GmpRat):{
                mpq_class* result = new mpq_class(asBigInt() * other.asBigRat());
                delete reinterpret_cast<mpz_class*>(data);
                data = result;
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(GmpRat, WordInt):
                asBigRat() *= (int32_t)other.asWordInt();
                if(reduce) bigRatReduce();
                break;
            case typePair(GmpRat, WordRat):
                asBigRat() *= other.asWordRat().num;
                asBigRat() /= other.asWordRat().den;
                if(reduce) bigRatReduce();
                break;
            case typePair(GmpRat, GmpInt):
                asBigRat() *= other.asBigInt();
                if(reduce) bigRatReduce();
                break;
            case typePair(GmpRat, GmpRat):
                asBigRat() *= other.asBigRat();
                if(reduce) bigRatReduce();
                break;
            default: assert(false);
        }
    }

    template<bool reduce = true>
    NumType operator*(const NumType& other) const{
        NumType ans(*this);
        ans.operator*=<reduce>(other);
        return ans;
    }

    template<bool reduce = true>
    void operator/=(const NumType& other){
        switch (other.type) {
            case WordInt:{
                int64_t z = other.asWordInt();
                operator*=( z>=0 ? NumType(1,z) : NumType(-1,-z) );
                break;
            }
            case WordRat:{
                rat64_t q = other.asWordRat();
                operator*=(mpq_class(q.den, q.num));
                break;
            }
            case GmpInt:
                operator*=(mpq_class(1,other.asBigInt()));
                break;
            case GmpRat:{
                mpq_class recip = 1/other.asBigRat();
                operator*=(recip);
                break;
            }
        }
    }

    template<bool reduce = true>
    NumType operator/(const NumType& other) const{
        NumType ans(*this);
        ans.operator/=<reduce>(other);
        return ans;
    }

    template<bool reduce = true>
    void operator+=(const NumType& other){
        switch(typePair(type, other.type)){
            case typePair(WordInt, WordInt):
                data = reinterpret_cast<void*>(asWordInt() + other.asWordInt());
                wordIntClamp();
                break;
            case typePair(WordInt, WordRat):{
                rat64_t ans;
                int32_t lhs = asWordInt();
                rat64_t rhs = other.asWordRat();
                if(rat64_t::add(lhs, rhs, ans)){
                    mpq_class* result = new mpq_class(rhs.num, rhs.den);
                    result->operator+=(lhs);
                    data = result;
                    type = GmpRat;
                    if(reduce) bigRatReduce();
                }else if(ans.den == 1){
                    data = reinterpret_cast<void*>(ans.num);
                }else{
                    data = ans;
                    type = WordRat;
                }
                break;
            }
            case typePair(WordInt, GmpInt):{
                data = new mpz_class(other.asBigInt()+(int32_t)asWordInt());
                type = GmpInt;
                if(reduce) bigIntReduce();
                break;
            }
            case typePair(WordInt, GmpRat):{
                data = new mpq_class(other.asBigRat()+(int32_t)asWordInt());
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(WordRat, WordInt):{
                rat64_t ans;
                rat64_t lhs = asWordRat();
                int32_t rhs = other.asWordInt();
                if(rat64_t::add(lhs, rhs, ans)){
                    mpq_class* result = new mpq_class(lhs.num, lhs.den);
                    result->operator+=(rhs);
                    data = result;
                    type = GmpRat;
                    if(reduce) bigRatReduce();
                }else if(ans.den == 1){
                    data = reinterpret_cast<void*>(ans.num);
                    type = WordInt;
                }else{
                    data = ans;
                }
                break;
            }
            case typePair(WordRat, WordRat):{
                rat64_t ans;
                rat64_t lhs = asWordRat();
                rat64_t rhs = other.asWordRat();
                if(rat64_t::add(lhs, rhs, ans)){
                    mpq_class* result = new mpq_class(lhs.num, lhs.den);
                    result->operator+=(mpq_class(rhs.num, rhs.den));
                    data = result;
                    type = GmpRat;
                    if(reduce) bigRatReduce();
                }else if(ans.den == 1){
                    data = reinterpret_cast<void*>(ans.num);
                    type = WordInt;
                }else{
                    data = ans;
                }
                break;
            }
            case typePair(WordRat, GmpInt):{
                mpq_class* next = new mpq_class(asWordRat().num, asWordRat().den);
                next->operator+=(other.asBigInt());
                data = next;
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(WordRat, GmpRat):{
                mpq_class* next = new mpq_class(other.asBigRat());
                next->operator+=(mpq_class(asWordRat().num, asWordRat().den));
                data = next;
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(GmpInt, WordInt):
                asBigInt() += (int32_t)other.asWordInt();
                if(reduce) bigIntReduce();
                break;
            case typePair(GmpInt, WordRat):{
                mpq_class* next = new mpq_class(other.asWordRat().num, other.asWordRat().den);
                next->operator+=(asBigInt());
                delete reinterpret_cast<mpz_class*>(data);
                data = next;
                type = WordRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(GmpInt, GmpInt):
                asBigInt() += other.asBigInt();
                if(reduce) bigIntReduce();
                break;
            case typePair(GmpInt, GmpRat):{
                mpq_class* result = new mpq_class(asBigInt() + other.asBigRat());
                delete reinterpret_cast<mpz_class*>(data);
                data = result;
                type = GmpRat;
                if(reduce) bigRatReduce();
                break;
            }
            case typePair(GmpRat, WordInt):
                asBigRat() += (int32_t)other.asWordInt();
                if(reduce) bigRatReduce();
                break;
            case typePair(GmpRat, WordRat):
                asBigRat() += mpq_class(other.asWordRat().num, other.asWordRat().den);
                if(reduce) bigRatReduce();
                break;
            case typePair(GmpRat, GmpInt):
                asBigRat() += other.asBigInt();
                if(reduce) bigRatReduce();
                break;
            case typePair(GmpRat, GmpRat):
                asBigRat() += other.asBigRat();
                if(reduce) bigRatReduce();
                break;
            default: assert(false);
        }
    }

    template<bool reduce = true>
    NumType operator+(const NumType& other) const{
        NumType ans(*this);
        ans.operator+=<reduce>(other);
        return ans;
    }

    template<bool reduce = true>
    void operator-=(const NumType& other){
        operator+=<reduce>(other.operator-<reduce>());
    }

    template<bool reduce = true>
    NumType operator-(const NumType& other) const{
        NumType ans(*this);
        ans.operator-=<reduce>(other);
        return ans;
    }

    template<bool reduce = true>
    void operator%=(const NumType& other){
        switch(typePair(type, other.type)){
            case typePair(WordInt, WordInt):
                data = reinterpret_cast<void*>(asWordInt() % other.asWordInt());
                break;
            case typePair(WordInt, WordRat):{
                rat64_t ans = other.asWordRat();
                ans.num = (asWordInt()*ans.num) % ans.den;
                data = ans;
                type = WordRat;
                wordRatReduce();
                break;
            }
            case typePair(WordInt, GmpInt):
                break;
            case typePair(WordInt, GmpRat):{
                mpq_class* ans = new mpq_class(other.asBigRat());
                ans->get_num() = (asWordInt()*ans->get_num()) % ans->get_den();
                data = ans;
                type = GmpRat;
                bigRatReduce();
                break;
            }
            case typePair(WordRat, WordInt):{
                rat64_t r = asWordRat();
                r.num %= (r.den*other.asWordInt());
                data = r;
                break;
            }
            case typePair(WordRat, WordRat):{
                rat64_t lhs = asWordRat();
                rat64_t rhs = other.asWordRat();
                int64_t den = lhs.den*rhs.den;
                int64_t num = lhs.num*rhs.den % (lhs.den*rhs.num);
                assert(den <= std::numeric_limits<uint32_t>::max()); //DO THIS - this can overflow
                assert(num <= std::numeric_limits<int32_t>::max() && num >= std::numeric_limits<int32_t>::min());
                data = rat64_t(num,den);
                wordRatReduce();
                break;
            }
            case typePair(WordRat, GmpInt):
                break;
            case typePair(WordRat, GmpRat):{
                rat64_t lhs = asWordRat();
                mpq_class& rhs = other.asBigRat();
                mpq_class* ans = new mpq_class(lhs.num*rhs.get_den() % (lhs.den * rhs.get_num()),
                                               rhs.get_den()*lhs.den);
                data = ans;
                type = GmpRat;
                bigRatReduce();
                break;
            }
            case typePair(GmpInt, WordInt):
                asBigInt() %= (int32_t)other.asWordInt();
                bigIntReduce();
                break;
            case typePair(GmpInt, WordRat):{
                mpz_class lhs = asBigInt();
                rat64_t rhs = other.asWordRat();
                mpq_class* ans = new mpq_class(lhs*rhs.den % rhs.num, rhs.den);
                data = ans;
                type = GmpRat;
                bigRatReduce();
                break;
            }
            case typePair(GmpInt, GmpInt):
                asBigInt() %= other.asBigInt();
                bigIntReduce();
                break;
            case typePair(GmpInt, GmpRat):{
                mpz_class lhs = asBigInt();
                mpq_class& rhs = other.asBigRat();
                mpq_class* ans = new mpq_class(lhs*rhs.get_den() % rhs.get_num(), rhs.get_den());
                data = ans;
                type = GmpRat;
                bigRatReduce();
                break;
            }
            case typePair(GmpRat, WordInt):{
                mpq_class& r = asBigRat();
                r.get_num() %= (r.get_den() * other.asWordInt());
                bigRatReduce();
                break;
            }
            case typePair(GmpRat, WordRat):{
                mpq_class& lhs = asBigRat();
                rat64_t rhs = other.asWordRat();
                lhs.get_den() *= rhs.den;
                lhs.get_num() = lhs.get_num()*rhs.den % (lhs.get_den()*rhs.num);
                bigRatReduce();
                break;
            }
            case typePair(GmpRat, GmpInt):{
                mpq_class& r = asBigRat();
                r.get_num() %= (r.get_den() * other.asBigInt());
                bigRatReduce();
                break;
            }
            case typePair(GmpRat, GmpRat):{
                mpq_class& lhs = asBigRat();
                mpq_class& rhs = other.asBigRat();
                lhs.get_den() *= rhs.get_den();
                lhs.get_num() = lhs.get_num()*rhs.get_den() % (lhs.get_den()*rhs.get_num());
                bigRatReduce();
                break;
            }
            default: assert(false);
        }
    }

    template<bool reduce = true>
    NumType operator%(const NumType& other) const{
        NumType ans(*this);
        ans.operator%=<reduce>(other);
        return ans;
    }
};

#endif // BIG_NUMERIC_SUM_TYPE_H
