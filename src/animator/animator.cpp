#include <animator/animator.h>

namespace animator
{
	const float Anim::Slow = 0.8f;
	const float Anim::Normal = 0.4f;
	const float Anim::Fast = 0.25f;
	const float Anim::DelayFast = 0.15f;

    AnimSequencerSequencial& Seq()
    { 
        AnimSequencerSequencial* pSequencerSequencial = new AnimSequencerSequencial();
        return *pSequencerSequencial;
    }

    AnimSequencerParallel& Parallel()
    {
        AnimSequencerParallel* pSequencerParallel = new AnimSequencerParallel();
        return *pSequencerParallel;
    }

	
	AnimSequencerAtomic& Wait(float fTime)
	{
		AnimWait* pWait = new AnimWait(fTime);
		AnimSequencerAtomic* pSequencerAtomic = new AnimSequencerAtomic(std::unique_ptr<PropertyBase>(pWait));
        pSequencerAtomic->_concatenable_vector.emplace_back( pSequencerAtomic );

        return *pSequencerAtomic;
	}
}
