#include <chrono>
#include <iostream>

#include "rat64_t.h"
#include "big_numeric_sum_type.h"

constexpr size_t benchmark_iters = 500000;

void benchmarkSumType(){
    std::cout << "SumType Integer Mult: ";
    auto start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < benchmark_iters; i++){
        NumType t = 1;
        for(size_t i = 0; i < 31; i++)
            t *= 2;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << duration.count() << "ms" << std::endl;

    std::cout << "SumType Rational Mult: ";
    start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < benchmark_iters; i++){
        NumType t(1,2);
        for(size_t i = 0; i < 31; i++)
            t *= NumType(1,2);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << duration.count() << "ms" << std::endl;

    std::cout << "SumType High Integer Mult: ";
    start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < benchmark_iters; i++){
        NumType t(1);
        for(size_t i = 0; i < 256; i++)
            t *= 2;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << duration.count() << "ms" << std::endl;
}

void benchmarkGmp(){
    std::cout << "mpq_class integer mult: ";
    auto start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < benchmark_iters; i++){
        mpq_class t = 1;
        for(size_t i = 0; i < 31; i++)
            t *= mpq_class(2);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << duration.count() << "ms" << std::endl;

    std::cout << "mpq_class rational mult: ";
    start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < benchmark_iters; i++){
        mpq_class t(1,2);
        for(size_t i = 0; i < 31; i++)
            t *= mpq_class(1,2);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << duration.count() << "ms" << std::endl;

    std::cout << "mpq_class high integer mult: ";
    start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < benchmark_iters; i++){
        mpq_class t(1);
        for(size_t i = 0; i < 256; i++)
            t *= mpq_class(2);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << duration.count() << "ms" << std::endl;
}

#include <math.h>

int main(){
    constexpr rat64_t::SignedHalfWord max_n = std::numeric_limits<int32_t>::max();
    constexpr rat64_t::SignedHalfWord min_n = std::numeric_limits<int32_t>::min()+1;
    constexpr rat64_t::UnsignedHalfWord max_d = std::numeric_limits<uint32_t>::max();

    rat64_t ans;

    assert( rat64_t(4,2) == rat64_t({2,1}) );
    assert( rat64_t(2,4) == rat64_t({1,2}) );
    assert( rat64_t(6,8) == rat64_t({3,4}) );

    assert( !rat64_t::multiply( rat64_t({1,2}), rat64_t({1,2}), ans ) );
    assert( ans == rat64_t({1,4}) );
    assert( !rat64_t::multiply( rat64_t({-1,2}), rat64_t({1,2}), ans ) );
    assert( ans == rat64_t({-1,4}) );
    assert( !rat64_t::multiply( rat64_t({-1,2}), rat64_t({-1,2}), ans ) );
    assert( ans == rat64_t({1,4}) );
    assert( !rat64_t::multiply( rat64_t({1,2}), rat64_t({2,3}), ans ) );
    assert( ans == rat64_t({1,3}) );
    assert( !rat64_t::multiply( rat64_t({-1,2}), rat64_t({2,3}), ans ) );
    assert( ans == rat64_t({-1,3}) );
    assert( !rat64_t::multiply( rat64_t({max_n,2}), rat64_t({2,1}), ans ) );
    assert( ans == rat64_t({max_n,1}) );
    assert( !rat64_t::multiply( rat64_t({min_n,2}), rat64_t({2,1}), ans ) );
    assert( ans == rat64_t({min_n,1}) );
    assert( !rat64_t::multiply( rat64_t({2,max_d}), rat64_t({1,2}), ans ) );
    assert( ans == rat64_t({1,max_d}) );
    assert( !rat64_t::multiply( rat64_t({max_n,1}), rat64_t({1,(uint32_t)max_n}), ans ) );
    assert( ans == rat64_t({1,1}) );
    assert( !rat64_t::multiply( rat64_t({min_n,1}), rat64_t({1,(uint32_t)-(int64_t)min_n}), ans ) );
    assert( ans == rat64_t({-1,1}) );

    assert( !rat64_t::multiply( rat64_t({1,2}), 3, ans ) );
    assert( ans == rat64_t({3,2}) );
    assert( !rat64_t::multiply( 3, rat64_t({1,2}), ans ) );
    assert( ans == rat64_t({3,2}) );
    assert( !rat64_t::multiply( rat64_t({1,2}), -3, ans ) );
    assert( ans == rat64_t({-3,2}) );
    assert( !rat64_t::multiply( -3, rat64_t({1,2}), ans ) );
    assert( ans == rat64_t({-3,2}) );

    assert( rat64_t::multiply( rat64_t({max_n,1}), rat64_t({2,1}), ans ) );
    assert( rat64_t::multiply( rat64_t({min_n,1}), rat64_t({2,1}), ans ) );
    assert( rat64_t::multiply( rat64_t({1,max_d}), rat64_t({1,2}), ans ) );

    assert( !rat64_t::add( rat64_t({1,2}), rat64_t({1,2}), ans ) );
    assert( ans == rat64_t({1,1}) );
    assert( !rat64_t::add( rat64_t({1,2}), rat64_t({1,3}), ans ) );
    assert( ans == rat64_t({5,6}) );
    assert( !rat64_t::add( rat64_t({1,2}), rat64_t({-1,3}), ans ) );
    assert( ans == rat64_t({1,6}) );
    assert( !rat64_t::add( rat64_t({5,3}), rat64_t({-9,10}), ans ) );
    assert( ans == rat64_t({23,30}) );
    assert( !rat64_t::add( rat64_t({-1,2}), rat64_t({-1,2}), ans ) );
    assert( ans == rat64_t({-1,1}) );
    assert( !rat64_t::add( rat64_t({-1,2}), 1, ans ) );
    assert( ans == rat64_t({1,2}) );
    assert( !rat64_t::add( rat64_t({-1,2}), -1, ans ) );
    assert( ans == rat64_t({-3,2}) );

    assert( !rat64_t::power( rat64_t({1,2}), 2, ans ) );
    assert( ans == rat64_t({1,4}) );
    assert( !rat64_t::power( rat64_t({1,2}), 4, ans ) );
    assert( ans == rat64_t({1,16}) );
    assert( rat64_t::power( rat64_t({1,2}), 32, ans ) );

    assert( rat64_t({1,4}) < rat64_t({1,3}) );
    assert( !(rat64_t({1,3}) < rat64_t({1,3})) );
    assert( !(rat64_t({1,4}) > rat64_t({1,3})) );
    assert( rat64_t({1,4}) <= rat64_t({1,3}) );
    assert( rat64_t({1,3}) <= rat64_t({1,3}) );

    assert( std::to_string(static_cast<double>(rat64_t{1,3})) == "0.333333" );

    assert( !rat64_t({31415926,10000000}).greaterThanPi() );
    assert( rat64_t({31415927,10000000}).greaterThanPi() );

    ans = rat64_t({1,2});
    assert( rat64_t(static_cast<void*>(ans)) == ans );
    ans = rat64_t({max_n, max_d});
    assert( rat64_t(static_cast<void*>(ans)) == ans );

    //Sum type tests
    NumType t = NumType(1)*NumType(2);
    assert(t.type == WordInt);
    assert(t.asWordInt() == 2);

    t *= 3;
    assert(t.type == WordInt);
    assert(t.asWordInt() == 6);

    t *= NumType(std::numeric_limits<int32_t>::max());
    assert(t.type == GmpInt);
    assert(t.asBigInt() == mpz_class(6)*mpz_class(std::numeric_limits<int32_t>::max()));

    t = NumType(1,2);
    t *= t;
    assert(t.type == WordRat);
    assert(t.asWordRat() == rat64_t({1,4}));
    t *= 4;
    assert(t.type == WordInt);
    assert(t.asWordInt() == 1);

    t = NumType(1);
    for(size_t i = 0; i < 31; i++)
        t *= NumType(1,2);
    assert(t.type == WordRat);
    assert(t.asWordRat() == rat64_t({1, 1u << 31}));
    t *= NumType(1,2);
    assert(t.type == GmpRat);
    assert(t.asBigRat().get_den().get_str() == std::to_string(1L << 32));

    t = NumType(1,3) + NumType(1,2);
    assert(t.type == WordRat);
    assert(t.asWordRat() == rat64_t({5,6}));
    t += NumType(1,6);
    assert(t.type == WordInt);
    assert(t.asWordInt() == 1);

    t = NumType(3,2) + NumType(3,2);
    assert(t.type == WordInt);
    assert(t.asWordInt() == 3);

    t = NumType(5) % NumType(3);
    assert(t.type == WordInt);
    assert(t.asWordInt() == 2);
    t = NumType(-5) % NumType(3);
    assert(t.type == WordInt);
    assert(t.asWordInt() == -2);
    t = NumType(5) % NumType(-3);
    assert(t.type == WordInt);
    assert(t.asWordInt() == 2);
    t = NumType(-5) % NumType(-3);
    assert(t.type == WordInt);
    assert(t.asWordInt() == -2);
    t = NumType(5,2) % NumType(2);
    assert(t.type == WordRat);
    assert(t.asWordRat() == rat64_t({1,2}));
    t = NumType(5) % NumType(2,3);
    assert(t.type == WordRat);
    assert(t.asWordRat() == rat64_t({1,3}));
    t = NumType(6) % NumType(2,3);
    assert(t.type == WordInt);
    assert(t.asWordInt() == 0);
    t = NumType(1,3) % NumType(1,4);
    assert(t.type == WordRat);
    assert(t.asWordRat() == rat64_t({1,12}));
    t = NumType(12,5) % NumType(7,5);
    assert(t.type == WordInt);
    assert(t.asWordInt() == 1);

    t = std::abs(NumType(-1));
    assert(t.type == WordInt);
    assert(t.asWordInt() == 1);
    t = std::abs(NumType(-1,2));
    assert(t.type == WordRat);
    assert(t.asWordRat() == rat64_t({1,2}));

    t = NumType(mpz_class("500000000")) * NumType(-100) + NumType(1)*NumType(mpz_class("-10000000000"));
    assert(t.toString() == "-60000000000");

    t = NumType(-100);
    assert(std::pow(t,0).toString() == "1");
    assert(std::pow(t,1).toString() == "-100");
    assert(std::pow(t,2).toString() == "10000");

    std::cout << "ALL TESTS PASSING" << std::endl;

    benchmarkSumType();
    benchmarkGmp();

    return 0;
}

/*
#include <gmpxx.h>
void findMaxFactorial(){
    for(size_t i = 10; true; i++){
        if( mpz_class::factorial(mpz_class(i)) > std::numeric_limits<int32_t>::max() ){
            std::cout << "The maximum int32_t factorial without overflow is " << i-1 << '!' << std::endl;
            break;
        }
    }

    for(size_t i = 10; true; i++){
        if( mpz_class::factorial(mpz_class(i)) > std::numeric_limits<int64_t>::max() ){
            std::cout << "The maximum int64_t factorial without overflow is " << i-1 << '!' << std::endl;
            break;
        }
    }
}
*/

/*
#include <gmpxx.h>
#include <math.h>
void boundPi(){
    std::string very_good_approximation = "3.1415926535897932384626433832795028841971693993751058"
                                          "209749445923078164062862089986280348253421170679821480"
                                          "865132823066470938446095505822317253594081284811174502"
                                          "841027019385211055596446229489549303819644288109756659"
                                          "334461284756482337867831652712019091456485669234603486"
                                          "104543266482133936072602491412737245870066063155881748"
                                          "815209209628292540917153643678925903600113305305488204"
                                          "665213841469519415116094330572703657595919530921861173"
                                          "819326117931051185480744623799627495673518857527248912"
                                          "279381830119491298336733624406566430860213949463952247"
                                          "371907021798609437027705392171762931767523846748184676"
                                          "694051320005681271452635608277857713427577896091736371"
                                          "787214684409012249534301465495853710507922796892589235"
                                          "420199561121290219608640344181598136297747713099605187"
                                          "072113499999983729780499510597317328160963185950244594"
                                          "553469083026425223082533446850352619311881710100031378"
                                          "387528865875332083814206171776691473035982534904287554"
                                          "687311595628638823537875937519577818577805321712268066"
                                          "130019278766111959092164201989380952572010654858632788"
                                          "659361533818279682303019520353018529689957736225994138"
                                          "912497217752834791315155748572424541506959508295331168"
                                          "617278558890750983817546374649393192550604009277016711"
                                          "390098488240128583616035637076601047101819429555961989"
                                          "467678374494482553797747268471040475346462080466842590"
                                          "694912933136770289891521047521620569660240580381501935"
                                          "112533824300355876402474964732639141992726042699227967"
                                          "823547816360093417216412199245863150302861829745557067"
                                          "498385054945885869269956909272107975093029553211653449"
                                          "872027559602364806654991198818347977535663698074265425"
                                          "278625518184175746728909777727938000816470600161452491"
                                          "921732172147723501414419735685481613611573525521334757"
                                          "418494684385233239073941433345477624168625189835694855"
                                          "620992192221842725502542568876717904946016534668049886"
                                          "272327917860857843838279679766814541009538837863609506"
                                          "800642251252051173929848960841284886269456042419652850"
                                          "222106611863067442786220391949450471237137869609563643"
                                          "719172874677646575739624138908658326459958133904780275"
                                          "900994657640789512694683983525957098258226205224894077"
                                          "267194782684826014769909026401363944374553050682034962"
                                          "524517493996514314298091906592509372216964615157098583"
                                          "874105978859597729754989301617539284681382686838689427"
                                          "741559918559252459539594310499725246808459872736446958"
                                          "486538367362226260991246080512438843904512441365497627"
                                          "807977156914359977001296160894416948685558484063534220"
                                          "722258284886481584560285060168427394522674676788952521"
                                          "385225499546667278239864565961163548862305774564980355"
                                          "936345681743241125150760694794510965960940252288797108"
                                          "931456691368672287489405601015033086179286809208747609"
                                          "178249385890097149096759852613655497818931297848216829";
    for(size_t i = 1; i < very_good_approximation.size()-1; i++)
        very_good_approximation[i] = very_good_approximation[i+1];
    very_good_approximation.pop_back();
    size_t decimals = very_good_approximation.size()-1;
    very_good_approximation += "/1";
    for(size_t i = 0; i < decimals; i++) very_good_approximation += "0";

    mpq_class pi(very_good_approximation);

    rat64_t low;
    rat64_t high;
    mpq_class low_error(-1);
    mpq_class high_error(1);

    for(uint64_t den = 1; den < std::numeric_limits<int32_t>::max(); den++){
        int64_t start = std::min((den*314159265358)/100000000000, (uint64_t)std::numeric_limits<int32_t>::max());
        if(start > std::numeric_limits<int32_t>::max()) break;
        int64_t end = std::min((den*314159265359)/100000000000, (uint64_t)std::numeric_limits<int32_t>::max());
        for(int64_t num = start; num <= end; num++){
            if( std::abs(static_cast<double>(num)/den - 3.14159265358979) > 1e-11 ) continue;

            mpq_class candidate(num, den);
            mpq_class error = candidate - pi;
            if(error > 0 && error < high_error){
                high_error = error;
                high = rat64_t(num, den);
            }else if(error < 0 && error > low_error){
                low_error = error;
                low = rat64_t(num, den);
            }
        }
    }

    std::cout << "The closest rat64_t greater than pi is " << high
              << " with an error of ~" << high_error.get_d()
              << "\nThe closest rat64_t less than pi is " << low
              << " with an error of ~" << -low_error.get_d() << std::endl;

    const double pi_double = 4*atan(1.0);
    std::string pi_double_str = std::to_string(pi_double);
    for(size_t i = 1; i < pi_double_str.size()-1; i++)
        pi_double_str[i] = pi_double_str[i+1];
    pi_double_str.pop_back();
    decimals = pi_double_str.size()-1;
    pi_double_str += "/1";
    for(size_t i = 0; i < decimals; i++) pi_double_str += "0";

    mpq_class err = mpq_class(pi_double_str) - pi;

    std::cout << "Compare with π-4*atan(1) with an error of ~" << err.get_d() << std::endl;
}
*/
