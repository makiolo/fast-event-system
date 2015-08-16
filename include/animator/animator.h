#ifndef _ANIM_H_
#define _ANIM_H_

#include <animator/interpolation.h>
#include <functional>
#include <vector>
#include <map>
#include <memory>

using uint = unsigned int;

namespace animator
{
    class PropertyBase
    {
        public:
            PropertyBase()
                : _erase( false )
                , _currenttime( 0.f )
                , _totaltime( 1.f )
            {}

            virtual ~PropertyBase() {}
            virtual bool Update( float dt ) = 0;

            void Time( float fTime )
			{
				_totaltime = fTime;
			}

        protected:
            bool        _erase;
            float       _totaltime;
            float       _currenttime;
    };

    template <typename TFunction, typename TParam>
    class AnimProperty : public PropertyBase
    {
        public:
			using TInterpolator = std::function<TParam(const TParam&, const TParam&, float)>;
			
            AnimProperty( TFunction tFunction, const TParam& tInitialParam, const TParam& tFinalParam, TInterpolator tInterpolator )
                : _func( tFunction )
                , _interpolator( tInterpolator )
                , _initial_param( tInitialParam )
                , _final_param( tFinalParam )
            {
				
            }

			bool Update( float dt )
			{
				float d = _currenttime / _totaltime;
				clamp( d, 0.f, 1.f );

				TParam tParam = _interpolator( _initial_param, _final_param, d );
				_func( tParam );

				if ( _currenttime > _totaltime )
					return false;
				
				_currenttime += dt;
				return true;
			}

        private:
			TFunction       _func;
			TInterpolator   _interpolator;
			TParam          _initial_param;
			TParam          _final_param;
    };

	class AnimWait : public PropertyBase
	{
	public:
		AnimWait (float fTime) { Time(fTime); }

		bool Update( float dt )
		{
			if ( _currenttime > _totaltime )
				return false;
				
			_currenttime += dt;
			return true;
		}
	};

	class AnimSequencer
	{
		public:
			using TAnimSequencerVector = std::vector<AnimSequencer*>;
			
			virtual ~AnimSequencer() { ; }
			virtual bool Update( float dt ) = 0;
			
			void AddSequencer( AnimSequencer* pSequencer )
			{
				_concatenable_vector.emplace_back(pSequencer);
			}
			
			AnimSequencer& operator,( AnimSequencer& rhs )
			{
				_concatenable_vector.emplace_back(&rhs);
				return *this;
			}
			
		public:
			TAnimSequencerVector _concatenable_vector;
	};

    class AnimSequencerAtomic;

    template <typename TFunction, class TParam>
    AnimSequencerAtomic& Prop( TFunction tFunction, const TParam& tInitialParam, const TParam& tFinalParam, typename AnimProperty<TFunction, TParam>::TInterpolator tInterpolator)
    {
		
		AnimSequencerAtomic* pSequencerAtomic = new AnimSequencerAtomic(std::make_unique<AnimProperty<TFunction, TParam> >(tFunction, tInitialParam, tFinalParam, tInterpolator));
        pSequencerAtomic->_concatenable_vector.emplace_back( pSequencerAtomic );

        return *pSequencerAtomic;
    }

    class AnimSequencerSequencial;
    AnimSequencerSequencial& Seq();

    class AnimSequencerParallel;
    AnimSequencerParallel& Parallel();

    class AnimSequencerAtomic : public AnimSequencer
    {
        public:
            AnimSequencerAtomic( std::unique_ptr<PropertyBase> pAnimProperty )
                : _anim_property( std::move(pAnimProperty) )
            {
				
            }

			bool Update( float dt )
			{
				return _anim_property->Update( dt );
			}

			AnimSequencerAtomic& Time( float fTime )
			{
				_anim_property->Time( fTime );
				return *this;
			}

        private:
			std::unique_ptr<PropertyBase>   _anim_property;
    };

    class AnimSequencerSequencial : public AnimSequencer
    {
        public:
            bool Update( float dt )
            {
                if ( _sequence_vector.empty() )
                    return false;

                AnimSequencer* seq = _sequence_vector.front();
                bool bContinue = seq->Update( dt );

                if ( !bContinue )
                {
                    delete seq;
                    _sequence_vector.erase( _sequence_vector.begin() );
                }

                return !_sequence_vector.empty();
            }

            AnimSequencerSequencial& operator[]( AnimSequencer& tVector )
            {
                _sequence_vector = tVector._concatenable_vector;
				_concatenable_vector.emplace_back(this);
                return *this;
            }
        private:
            TAnimSequencerVector    _sequence_vector;
    };


    class AnimSequencerParallel : public AnimSequencer
    {
        public:
            bool Update( float dt )
            {
                 if ( _sequence_vector.empty() )
                    return false;

                 bool bContinue = false;

                 for ( uint i=0; i<_sequence_vector.size(); ++i )
                 {
                     AnimSequencer* pSequencer = _sequence_vector[i];
                     bContinue = pSequencer->Update( dt ) || bContinue;
                 }

                 if ( !bContinue )
                 {
                     while ( !_sequence_vector.empty() )
                     {
                         delete _sequence_vector.back();
                         _sequence_vector.pop_back();
                     }
                 }

                 return bContinue;
            }

            AnimSequencerParallel& operator[]( AnimSequencer& tVector )
            {
                _sequence_vector = tVector._concatenable_vector;
				_concatenable_vector.emplace_back(this);
                return *this;
            }

        private:
            TAnimSequencerVector    _sequence_vector;
    };

	AnimSequencerAtomic& Wait(float fTime);

	class Anim : public std::enable_shared_from_this<Anim>
    {
        public:
			static const float Slow /*= 0.8f*/;
			static const float Normal /*= 0.4f*/;
			static const float Fast /*= 0.25f*/;
			static const float DelayFast /*= 0.15f*/;

			using Handler = int;
			using TAnimCallback = std::function<void()>;
			using THandlerVector = std::vector<Handler>;
            using TAnimMap = std::map<Handler, Anim* >;

			AnimSequencerParallel& Create( TAnimCallback tCallback, Handler* handler = NULL )
			{
				Handler tHandler = ++_next_handler;

				if ( _next_handler > 65535 )
					_next_handler = 1;

				if ( handler )
					*handler = tHandler;

				Anim* pAnim = new Anim( tHandler, tCallback );
				return pAnim->_root_seq;
			}

			AnimSequencerParallel& Create( Handler* pHandler = NULL )
            {
				Handler tHandler = ++_next_handler;

				if ( _next_handler > 65535 )
					_next_handler = 1;

				if ( pHandler )
					*pHandler = tHandler;

				Anim* pAnim = new Anim( tHandler );
                return pAnim->_root_seq;
            }

			void DestroyAnimation( Handler tHandler )
			{
				_anim_to_destroy.emplace_back(tHandler);
			}

            void DestroyAllAnimations()
            {
                _destroy_all_animations = true;
            }

            void Update( float dt )
            {
				if ( _destroy_all_animations )
				{
					_anim_to_destroy.clear();
					_anim_map.clear();
					_destroy_all_animations = false;
				}

				while( !_anim_to_destroy.empty() )
				{
					Handler tHandler = _anim_to_destroy.back();
					_anim_to_destroy.pop_back();

					if ( _anim_map.count( tHandler ) )
					{
						auto& anim = _anim_map[tHandler];
						_anim_map.erase( tHandler );
					}
				}

                if ( _anim_map.empty() )
                    return;

                auto it = _anim_map.begin();
                for ( ; it != _anim_map.end(); )
                {
                    auto& anim = it->second;
                    bool bContinue = anim->_root_seq.Update( dt );

                    if ( !bContinue )
                    {
						if ( anim->_has_callback )
							anim->_callback();
						
						it = _anim_map.erase( it );
					}
					else
					{
						++it;
					}
				}
			}
			
			Anim( Handler tHandler )
				: _handler( tHandler )
				, _has_callback( false )
			{
				_anim_map[tHandler] = this;
			}

			Anim( Handler tHandler, TAnimCallback tCallback )
				: _handler( tHandler )
				, _has_callback( true )
				, _callback( tCallback )
			{
				_anim_map[tHandler] = this;
			}
			
		private:
			AnimSequencerParallel _root_seq;
			bool _has_callback;
			TAnimCallback _callback;
			Handler _handler;
			bool _destroy_all_animations;
			Handler _next_handler;
			THandlerVector _anim_to_destroy;
            TAnimMap _anim_map;
    };

}

//--------------------------------------------------------------------------------------------
//
// FXFAnim es un sistema de animación genérico. La propiedad a animar la decide el usuario.
// Un ejemplo de uso sería el siguiente:
//
// - using namespace FXFAnim;
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
//	CFXFAnimSequencerAtomic& Prop( TFunction tFunction, const TParam& tInitialParam, const TParam& tFinalParam, typename TAnimProperty<TFunction, TParam>::TInterpolator tInterpolator = g_DefaultInterpolation )
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