#pragma once

#include <type_traits>
#include <iostream>
#include <cassert>
#include <array>
#include <math.h>

namespace mope
{
    /*========================================================================*\
    |  _base:                                                                  |
    |  Contains operations common to both vectors and matrices                 |
    |  (a vector is just a 1xN matrix)                                         |
    \*========================================================================*/
    /*
        N		The dimension of the vector (number of columns for a matrix).
        T		The inner type of the elements (column vectors for a matrix).
        Vec     The derived type.
    */
    namespace
    {
        template <size_t N, typename T, template <size_t, typename> class Vec>
        struct _base
        {
            T elements[N] = { };

            constexpr _base() = default;

            // subscript operator
            constexpr T& operator [] (const size_t& i)
            {
                return elements[i];
            }
            constexpr const T& operator [] (const size_t& i) const
            {
                return elements[i];
            }

            // element-wise addition
            template <typename S>
            constexpr auto operator + (const Vec<N, S>& rhs) const
            {
                Vec<N, std::common_type_t<T, S>> res;
                for (size_t i = 0; i < N; ++i)
                    res[i] = (*this)[i] + rhs[i];
                return res;
            }

            // element-wise subtraction
            template <typename S>
            constexpr auto operator - (const Vec<N, S>& rhs) const
            {
                Vec<N, std::common_type_t<T, S>> res;
                for (size_t i = 0; i < N; ++i)
                    res[i] = (*this)[i] - rhs[i];
                return res;
            }

            // negation
            constexpr auto operator - () const
            {
                Vec<N, T> res;
                for (size_t i = 0; i < N; ++i)
                    res[i] = -(*this)[i];
                return res;
            }

            // scalar multiplication
            template <typename S>
            constexpr auto operator * (S rhs) const
            {
                Vec<N, std::common_type_t<T, S>> res;
                for (size_t i = 0; i < N; ++i)
                    res[i] = (*this)[i] * rhs;
                return res;
            }

            // assignments
            template <typename S>
            constexpr auto& operator += (const Vec<N, S>& rhs)
            {
                for (size_t i = 0; i < N; ++i)
                    (*this)[i] += rhs[i];
                return *this;
            }

            template <typename S>
            constexpr auto& operator += (const std::initializer_list<S>& L)
            {
                static_assert(L.size() <= N);
                size_t i = 0;
                for (auto iter = L.begin(); iter != L.end(); iter++)
                    (*this)[++i] += *iter;
                return *this;
            }

            template <typename S>
            constexpr auto& operator -= (const Vec<N, S>& rhs)
            {
                for (size_t i = 0; i < N; ++i)
                    (*this)[i] -= rhs[i];
                return *this;
            }

            template <typename S>
            constexpr auto& operator -= (const std::initializer_list<S>& L)
            {
                static_assert(L.size() <= N);
                size_t i = 0;
                for (auto iter = L.begin(); iter != L.end(); iter++)
                    (*this)[++i] -= *iter;
                return *this;
            }

            constexpr auto& operator *= (float rhs)
            {
                for (size_t i = 0; i < N; ++i)
                    (*this)[i] *= rhs;
                return *this;
            }

            // equality
            constexpr bool operator == (const Vec<N, T>& other) const
            {
                return !memcmp(this->elements, other.elements, N * sizeof(T));
            }
            constexpr bool operator != (const Vec<N, T>& other) const
            {
                return !(*this == other);
            }
        };
    }

    // Scalar left-multiplication. Also explicitly instantiates _base::operator* for valid scalar types.
    template <size_t N, typename T, template <size_t, typename> class Vec>
    constexpr auto operator * (int lhs, const _base<N, T, Vec>& rhs)
    {
        return rhs * lhs;
    }

    template <size_t N, typename T, template <size_t, typename> class Vec>
    constexpr auto operator * (float lhs, const _base<N, T, Vec>& rhs)
    {
        return rhs * lhs;
    }

    template <size_t N, typename T, template <size_t, typename> class Vec>
    constexpr auto operator * (double lhs, const _base<N, T, Vec>& rhs)
    {
        return rhs * lhs;
    }

    
    /*========================================================================*\
    |  vec                                                                     |
    \*========================================================================*/
    template <size_t N, typename T> struct vec;
    namespace
    {
        template <size_t N, typename T>
        struct _vec : public _base<N, T, vec>
        {
            // implicit cast to higher dimensions
            template <size_t P>
            constexpr operator vec<P, T>() const
            {
                static_assert(P >= N, "Can't cast vector to a lower dimension.");

                vec<P, T> res;
                for (size_t i = 0; i < N; ++i)
                    res[i] = (*this)[i];
                return res;
            }

            // explicit cast to different data type / size
            template <size_t P, typename S>
            constexpr explicit operator vec<P, S>() const
            {
                static_assert(P >= N, "Can't cast vector to a lower dimension.");

                vec<P, S> res;
                for (size_t i = 0; i < N; ++i)
                    res[i] = static_cast<S>((*this)[i]);
                return res;
            }

            // dot product
            template <typename S>
            constexpr auto dot(const vec<N, S>& rhs) const
            {
                std::common_type_t<T, S> res = 0;
                for (size_t i = 0; i < N; ++i)
                    res += (*this)[i] * rhs[i];
                return res;
            }

            // cross product
            template <typename S>
            constexpr auto cross(const vec<N, S>& rhs) const
            {
                static_assert(N > 2, "Cross product is not defined for n < 3.");

                vec<N, std::common_type_t<T, S>> res;
                for (size_t i = 0; i < N - 2; ++i)
                    res[i] = (*this)[i + 1] * rhs[i + 2] - (*this)[i + 2] * rhs[i + 1];
                res[N - 2] = (*this)[N - 1] * rhs[0] - (*this)[0] * rhs[N - 1];
                res[N - 1] = (*this)[0] * rhs[1] - (*this)[1] * rhs[0];
                return res;
            }

            // elementwise scaling
            template <typename S>
            constexpr auto scaleby(const vec<N, S>& v) const
            {
                vec< N, std::common_type_t<T, S>> res;
                for (size_t i = 0; i < N; ++i)
                {
                    res[i] = (*this)[i] * v[i];
                }
                return res;
            }

            // magnitude
            double magnitude() const
            {
                double res = 0;
                vec<N, double> v = static_cast<vec<N, double>>(*this);
                for (size_t i = 0; i < N; ++i)
                    res += v[i] * v[i];
                return sqrt(res);
            }

            // magnitude as a float
            float fmagnitude() const
            {
                float res = 0;
                vec<N, float> v = static_cast<vec<N, float>>(*this);
                for (size_t i = 0; i < N; ++i)
                    res += v[i] * v[i];
                return sqrt(res);
            }

            // unit vector
            vec<N, double> unit() const
            {
                double factor = 1.0 / this->magnitude();
                return (*this) * factor;
            }

            // unit vector as floats
            vec<N, float> funit() const
            {
                float factor = 1.f / this->fmagnitude();
                return (*this) * factor;
            }
        };
    }
    
    
    /*========================================================================*\
    |  vec --- specializations                                                 |
    \*========================================================================*/
    template <class T>
    struct vec<2, T> : public _vec<2, T>
    {
        constexpr vec()
        {
        }
        constexpr vec(const T& x, const T& y)
        {
            (*this)[0] = x;
            (*this)[1] = y;
        }

        constexpr T& x() { return (*this)[0]; }
        constexpr T& y() { return (*this)[1]; }
        constexpr const T& x() const { return (*this)[0]; }
        constexpr const T& y() const { return (*this)[1]; }
    };

    template <class T>
    struct vec<3, T> : public _vec<3, T>
    {
        constexpr vec()
        {
        }
        constexpr vec(const T& x, const T& y, const T& z)
        {
            (*this)[0] = x;
            (*this)[1] = y;
            (*this)[2] = z;
        }

        constexpr T& x() { return (*this)[0]; }
        constexpr T& y() { return (*this)[1]; }
        constexpr T& z() { return (*this)[2]; }
        constexpr const T& x() const { return (*this)[0]; }
        constexpr const T& y() const { return (*this)[1]; }
        constexpr const T& z() const { return (*this)[2]; }
    };

    template <class T>
    struct vec<4, T> : public _vec<4, T>
    {
        constexpr vec()
        {
        }
        constexpr vec(const T& x, const T& y, const T& z, const T& w)
        {
            (*this)[0] = x;
            (*this)[1] = y;
            (*this)[2] = z;
            (*this)[3] = w;
        }
        constexpr vec<4, T>(const vec<4, T>& other)
        {
            memcpy(this->elements, other.elements, 4 * sizeof(T));
        }
        constexpr vec<4, T>& operator=(const vec<4, T>& other)
        {
            memcpy(this->elements, other.elements, 4 * sizeof(T));
            return *this;
        }

        constexpr T& x() { return (*this)[0]; }
        constexpr T& y() { return (*this)[1]; }
        constexpr T& z() { return (*this)[2]; }
        constexpr T& w() { return (*this)[3]; }
        constexpr const T& x() const { return (*this)[0]; }
        constexpr const T& y() const { return (*this)[1]; }
        constexpr const T& z() const { return (*this)[2]; }
        constexpr const T& w() const { return (*this)[3]; }
    };

    template <size_t N, class T>
    struct vec : public _vec<N, T>
    { };


    /*========================================================================*\
    |  mat                                                                    |
    \*========================================================================*/
    template <size_t N, typename V> struct mat;
    namespace
    {
        template <size_t M, size_t N, typename T>
        struct _mat : public _base<N, vec<M, T>, mat>
        {
            using _base<N, vec<M, T>, mat>::operator*;

            // matrix - vector multiplication
            template <typename S>
            constexpr auto operator * (const vec<N, S>& rhs) const
            {
                vec<M, std::common_type_t<T, S>> res;
                for (size_t i = 0; i < M; ++i)
                    for (size_t j = 0; j < N; ++j)
                        res[i] += (*this)[j][i] * rhs[j];
                return res;
            }

            // matrix - matrix multiplication
            template <size_t P, typename S>
            constexpr auto operator * (const mat<P, vec<N, S>>& rhs) const
            {
                mat<P, vec<M, std::common_type_t<T, S>>> res;
                for (size_t i = 0; i < M; ++i)
                    for (size_t j = 0; j < N; j++)
                        for (size_t k = 0; k < P; k++)
                            res[k][i] += (*this)[j][i] * rhs[k][j];
                return res;
            }

            // transpose
            constexpr auto transpose() const
            {
                mat<M, vec<N, T>> res;
                for (size_t i = 0; i < M; ++i)
                    for (size_t j = 0; j < N; ++j)
                        res[i][j] = (*this)[j][i];
                return res;
            }

            // get all the column-ordered elements as a vector
            constexpr auto unpack() const
            {
                std::array<T, M * N> dat;
                for (size_t j = 0; j < N; ++j)
                    for (size_t i = 0; i < M; ++i)
                        dat[j * N + i] = (*this)[j][i];
                return dat;
            }
        };
    }

    template <size_t M, size_t N, typename T>
    struct mat<N, vec<M, T>> : public _mat<M, N, T>
    { 
        constexpr mat()
        {
        }
        constexpr mat(const std::initializer_list<vec<M, T>>& L)
        {
            assert(L.size() <= N);
            std::copy(L.begin(), L.end(), this->elements);
        }
        constexpr mat(const std::initializer_list<T>& L)
        {
            assert(L.size() <= N * M);
            auto iter = L.begin();
            for (size_t idx = 0; idx < N; idx++) {
                std::copy(iter, std::next(iter, M), (*this)[idx].elements);
                std::advance(iter, M);
            }
        }

        using SquareMatrix = typename std::enable_if<M == N, mat<N, vec<N, T>>>::type;
        constexpr static SquareMatrix identity()
        {
            mat<N, vec<N, T>> res{};
            for (size_t i = 0; i < N; ++i)
                res[i][i] = static_cast<T>(1);
            return res;
        }
    };


    // allow for implicit type promotion in vec operations
    template <size_t N, typename T1, typename T2>
    struct std::common_type<vec<N, T1>, vec<N, T2>> {
        using type = mope::vec<N, std::common_type_t<T1, T2>>;
    };
    template <size_t N, typename T1, typename T2>
    struct std::common_type<vec<N, T1>, T2> {
        using type = mope::vec<N, std::common_type_t<T1, T2>>;
    };
    template <size_t N, typename T1, typename T2>
    struct std::common_type<T1, vec<N, T2>> {
        using type = mope::vec<N, std::common_type_t<T1, T2>>;
    };


    // stream printing 
    template <size_t N, typename T, template <size_t, typename> class Vec>
    std::ostream& operator<<(std::ostream& os, const _base<N, T, Vec>& v) {
        os << "( ";
        for (size_t i = 0; i < N; ++i) {
            os << v[i] << " ";
        }
        os << ")";
        return os;
    }
    template <size_t N, typename T, template <size_t, typename> class Vec>
    std::wostream& operator<<(std::wostream& os, const _base<N, T, Vec>& v) {
        os << "( ";
        for (size_t i = 0; i < N; ++i) {
            os << v[i] << " ";
        }
        os << ")";
        return os;
    }

    typedef vec<2, int>				vec2i;
    typedef vec<3, int>				vec3i;
    typedef vec<4, int>				vec4i;
    typedef vec<2, float>			vec2f;
    typedef vec<3, float>			vec3f;
    typedef vec<4, float>			vec4f;
    typedef vec<2, double>			vec2d;
    typedef vec<3, double>			vec3d;
    typedef vec<4, double>			vec4d;
    typedef vec<2, uint32_t>    	vec2ui;
    typedef vec<3, uint32_t>    	vec3ui;
    typedef vec<4, uint32_t>	    vec4ui;
    typedef vec<2, uint8_t>			vec2b;
    typedef vec<3, uint8_t>			vec3b;
    typedef vec<4, uint8_t>			vec4b;

    typedef mat<2, vec2i>			mat2i;
    typedef mat<3, vec3i>			mat3i;
    typedef mat<4, vec4i>			mat4i;
    typedef mat<2, vec2f>			mat2f;
    typedef mat<3, vec3f>			mat3f;
    typedef mat<4, vec4f>			mat4f;
    typedef mat<2, vec2d>			mat2d;
    typedef mat<3, vec3d>			mat3d;
    typedef mat<4, vec4d>			mat4d;
    typedef mat<2, vec2ui>			mat2ui;
    typedef mat<3, vec3ui>			mat3ui;
    typedef mat<4, vec4ui>			mat4ui;
} // namespace mope