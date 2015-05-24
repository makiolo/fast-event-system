#ifndef _INTERPOLATIONS_H_
#define _INTERPOLATIONS_H_

#define _USE_MATH_DEFINES
#include <math.h>

template <class T> T lerp( T A, T B, float d);
template <class T> float ilerp( T A, T B, T C );
template <class T> T smoothstep( T A, T B, float d );
template <class T> T smoothstep_squared( T A, T B, float d );
template <class T> T smoothstep_cubed( T A, T B, float d );
template <class T> T slow_acceleration_squared_interp( T A, T B, float d );
template <class T> T slow_decelleration_squared_interp( T A, T B, float d );
template <class T> T slow_acceleration_cubed_interp( T A, T B, float d );
template <class T> T slow_decelleration_cubed_interp( T A, T B, float d );
template <class T> T weighted_average_interp( T A, T B, T current );
template <class T> T sin_interp( T A, T B, float d );
template <class T> T catmull_rom_interp( T A, T B, float d, T q, T t );

template <typename T>
inline float clamp(T x, T a, T b)
{
	return x < a ? a : (x > b ? b : x);
}

template <class T>
T lerp( T A, T B, float d)
{
    clamp( d, 0.f, 1.f );
	T x = (T)(B * d + A * ( 1 - d ));
	
	return x;
}

template <class T>
float ilerp( T A, T B, T C )
{
    T a( A );
    T b( B );

    return lerp( 0.f, 1.f, C / ( B - A ) );
}

template <class T>
T smoothstep( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	d = d * d * (3 - 2 * d);
	clamp( d, 0.f, 1.f );
	
	T X = ( B * d ) + ( A * ( 1 - d ) );
	
	return X;
}

template <class T>
T smoothstep_squared( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	
	return smoothstep( A, B, d * d );
}

template <class T>
T smoothstep_cubed( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	
	return smoothstep( A, B, d * d * d );
}


template <class T>
T slow_acceleration_squared_interp( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	
	d = d * d;
	T X = B * d + A * ( 1 - d );
	
	return X;
}

template <class T>
T slow_decelleration_squared_interp( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	
	d = 1 - ( 1 - d ) * ( 1 - d );
	T X = B * d + A * ( 1 - d );
	
	return X;
}

template <class T>
T slow_acceleration_cubed_interp( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	
	d = d * d * d;
	
	T X = B * d + A * ( 1 - d );
	
	return X;
}

template <class T>
T slow_decelleration_cubed_interp( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	
	d = 1 - ( 1 - d ) * ( 1 - d ) * ( 1 - d );
	
	T X = B * d + A * ( 1 - d );
	
	return X;
}

template <class T>
T weighted_average_interp( T A, T B, T current )
{
	T X = ((current * (2 - 1)) + B) / 2;
	
	return X;
}

template <class T>
T sin_interp( T A, T B, float d )
{
	clamp( d, 0.f, 1.f );
	
	d = d * M_PI * 0.5f;
	
	T X = B * d + A * ( 1 - d );
	
	return X;
}

template <class T>
T catmull_rom_interp( T A, T B, float d, T q, T t )
{
	clamp( d, 0.f, 1.f );
	
	T p0 = q;
	T p1 = 0;
	T p2 = 1;
	T p3 = t;
	
	T catmullValue =  0.5f * (
							  (2 * p1) + 
							  (-p0 + p2) * d + 
							  (2 * p0 - 5 * p1 + 4 * p2 - p3) * d * d + 
							  (-p0 + 3 * p1 - 3 * p2 + p3) * d * d * d 
							  ); 
	
	d = catmullValue;
	
	T X = B * d + A * ( 1 - d );
	
	return X;
}

#endif // _INTERPOLATIONS_H_
