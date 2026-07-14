#pragma once
#include "state.h"

namespace oscillator {

   class HarmonicOscillator {
   public:
       explicit HarmonicOscillator(double angular_frequency);

       double angular_frequency() const;

       State exact_state(double time, const State& initial_state) const;
       State derivative(const State& state) const;


    private:
       double angular_frequency_;
   };

}
