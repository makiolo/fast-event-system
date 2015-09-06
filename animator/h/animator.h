#ifndef _ANIM_H_
#define _ANIM_H_

//--------------------------------------------------------------------------------------------
//
// - using namespace Anim;
// - Anim::Create()
// - [
// -	Prop( UI::Pos( this ), CFXEVector2( 0.f, 0.f ), CFXEVector2( 100.f, 0.f ) ).Time( 3.f )
// - ];
//
// Con lo anterior se animaría la propiedad CFXEVector2, que en este caso es una posición. Además,
// la animación durará 3 segundos
//
// Prop tiene la siguiente sintaxis:
//
//	template <typename TFunction, class TParam>
//	CAnimSequencerAtomic& Prop( TFunction tFunction, const TParam& tInitialParam, const TParam& tFinalParam, typename TAnimProperty<TFunction, TParam>::TInterpolator tInterpolator = g_DefaultInterpolation )
//
//	- tFunction debe tener implementado el operator() con un parámetro de tipo const TParam&, es decir, con la sintaxis: void operator()( const TParam& ),
//		el cual se irá llamando con el valor animado
//
//	- tInitialParam es el valor inicial de la animación
//	- tFinalParam es el valor final de la animación
//
//	- tInterpolator es la functión de interpolación de la animación, debe implementar un método, función, con la sintaxis "TParam operator( const TParam& tA, const TParam& tB, float fD )",
//		donde fD es un valor entre 0.f y 1.f, tA es el valor inicial y tB es el valor final.
//		Por defecto es una interpolación cúbica
//
//
// Prop solo puede animar una propiedad, pero una animación puede animar varias propiedades de forma secuencial o paralela. En el ámbito por defecto, las propiedades se animan de forma paralela.
// Un ejemplo de dos propiedades siendo animadas de forma paralela sería este:
//
// - Anim::Create()
// - [
// -	Prop( UI::Pos( this ), CFXEVector2( 0.f, 0.f ), CFXEVector2( 100.f, 0.f ) ).Time( 3.f ),
// -	Prop( UI::Color( this ), CFXEColor( 1.f, 0.f, 0.f ), CFXEColor( 0.f, 1.f, 0.f ) ).Time( 1.f )
// - ];
//
// Usando Seq(), se pueden concatenar animaciones en forma de secuencia:
//
// - Anim::Create()
// - [
// -	Seq()
// -	[
// -		Prop( UI::Pos( this ), CFXEVector2( 0.f, 0.f ), CFXEVector2( 100.f, 0.f ) ).Time( 3.f ),
// -		Prop( UI::Pos( this ), CFXEVector2( 100.f, 0.f ), CFXEVector2( 0.f, 0.f ) ).Time( 1.f )
// -	]
// - ];
//
//	Esta animación sería ir del punto 0,0 al 100,0 en 3 segundos y seguidamente ir desde esa misma posición al punto 0,0 en 1 segundo.
//
//
// En una animación se pueden concatenar secuencias y paralelas, en cualquier orden.
//
// - Anim::Create()
// - [
// -	Seq()
// -	[
// -		Prop( UI::Pos( this ), CFXEVector2( 0.f, 0.f ), CFXEVector2( 100.f, 0.f ) ).Time( 0.5f ),
// -		Prop( UI::Pos( this ), CFXEVector2( 100.f, 0.f ), CFXEVector2( 0.f, 0.f ) ).Time( 0.5f )
// -	],
// -	Prop( UI::Color( this ), CFXEColor( 1.f, 0.f, 0.f ), CFXEColor( 0.f, 1.f, 0.f ) ).Time( 2.f )
// - ];
//
// En el ejemplo anterior el primer par de corchetes [] se ejecuta simultáneamente, hemos dicho que en el ámbito por defecto ( el primer par de corchetes ) es paralelo.
// Luego tenemos un grupo secuencial que tiene una serie de cambios de posiciones.
// La animación será por tanto, de forma paralela se desplazará y cambiará de color. El desplazamiento se desplazará de 0 a 100 y luego de 100 a 0.
//
// Las animaciones se pueden cancelar de forma retardada utilizando un Handler. Ejemplo:
//
// - Handler tMyHandler;
// - Anim::Create( &tMyHandler )
// - [
// -	...
// - ];
//
//	//--- Cancela la animación
//	Anim::DestroyAnimation( tMyHandler );
//
// También se puede asociar un callback para que sea llamado al finalizar una animación, tiene la sintaxis: 'void ()', no recibe nada y devuelve void, puede ser una función, método, functor, etc.
// Si para el usuario es una limitación que no reciba ningún parámetro, siempre se puede optar por la currificación, mediante el empleo de std::bind
//
//
// ---------
//   ANEXO
// ---------
//
//	-	Los valores Anim::Slow, Anim::Normal y Anim::Fast definen los tiempos más comunes para las animaciones del UI, y es recomendable usarlos en lugar
//		de tiempos personalizados, para simplificar hacer cambios futuros.
//
//	-	Para simplificar el uso en el UI, se han definido una serie de functores para las propiedades más comunes de los elementos en el namespace UI::
//
//	-	Adicionalmente, se pueden usar las animaciones predefinidas para el UI, que permiten escribir menos código a la hora de hacer fundidos y animaciones
//		comunes de entrada y salida de elementos. Por ejemplo, estos códigos serían equivalentes:
//
//			Anim::Create()
//			[
//				Prop( UI::Alpha( pElem ), 1.f, 0.f ).Time( 2.f )
//			];
//
//			Anim::Create()
//			[
//				FadeOut( pElem ).Time( 2.f )
//			];
//
//
//--------------------------------------------------------------------------------------------

#endif // _ANIM_H_
