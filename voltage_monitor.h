// -*- mode: C++; -*-
/**\file voltage_monitor.h
\brief Trivial co-routine to gather and filter ADC samples
 */
#pragma once
#include <cstdlib>
#include <limits>
#include "Time.h"

/// Size of ring buffer for samples
const static size_t N = 2000;

/// Number of histogram bins
const static size_t K = 100;

/// Calibration data:
///    39930 19600
///    33957 16700
///    10190 5330
///    8816 5000
///    7984 3330
///
/// Regression equation:
///    0.4874166181 * x + 0.1581747627
const static float gain = 0.4874;
const static float offset = 0.1582;

///\class VoltageMonitor
///\brief Computes the median of 2000 samples with the work spread over
///  many small steps to maintain a responsive user interface.
class VoltageMonitor {
public:
   VoltageMonitor() : ring_index(0), sample_count(0), mv_hold(999999) {};

   /// Return the latest voltage reading.
   long getValue() {
      // Limit update rate to once per second.
      if (now() != updateTime) {
         update();
      }
      return mv_hold;
   }

   /// Keep the most recent N samples in a ring buffer.
   void getSample() {
      float raw(analogRead(analogInput));

      // Raw reading at 2.7V. Can't really run under this voltage.
      const float rawMin(0*12352.2);
      // Raw reading at 20V. Risking damage over this voltage.
      const float rawMax(10*112237.0);
      if (rawMin < raw && raw < rawMax) {
         sample[ring_index] = raw;
         ++ring_index;
         ++sample_count;
         if (N < sample_count) {
            // Saturate. There are N values in the buffer.
            sample_count = N;
         }
         if (ring_index == N) {
            // Wrap to keep most recent N.
            ring_index = 0;
         }
      } else {
         // Unreasonable ADC reading
         ;
      }
   }
   uint32_t getRawVoltage() {
      return analogRead(analogInput);
   }

   /// Filter and scale raw ADC values to millivolts.
   void update() {
      // Simple sum over the samples
      float sample_sum(0.0);

      // Discarding impossible values is done while gathering samples
      // in getSample() above.
      float min(std::numeric_limits<int>::max());
      float max(std::numeric_limits<int>::min());

      for (size_t k(0); k < sample_count; ++k) {
         sample_sum += sample[k];
         if (sample[k] < min) min = sample[k];
         if (max < sample[k]) max = sample[k];
      }

      float range(max - min);
      float bin(range/(K - 1));

      for (uint32_t k(0); k < K; ++k) {
         histogram[k] = 0.0;
         binsum[k] = 0.0;
      }

      for (uint32_t k(0); k < sample_count; ++k) {
         size_t index((sample[k] - min)/bin);
         if (0 <= index && index < K) {
            histogram[index]++;
            binsum[index] += sample[k];
         }
      }

      // Find the bottom quartile.
      size_t lower_quartile(0);
      for (uint32_t k(0);
           k < sample_count/4;
           k += histogram[lower_quartile++])
         ;

      // Find the top quartile.
      size_t upper_quartile(K);
      for (uint32_t k(0);
           k < sample_count/4;
           k += histogram[--upper_quartile])
         ;

      // Sum and count the middle two quartiles.
      float mid_sum(0.0);
      size_t mid_count(0.0);
      for (uint32_t k(lower_quartile); k <= upper_quartile; ++k) {
         mid_sum += binsum[k];
         mid_count += histogram[k];
      }

      if (0.0 < mid_count) {
         mv_hold = gain * mid_sum / mid_count + offset;
      } else {
         mv_hold = gain * sample_sum / sample_count + offset;
      }

      updateTime = now();
   }
private:
   /// The time in seconds of the last update.
   time_t updateTime;

   /// The index into the sample[] ring buffer.
   size_t ring_index;

   /// The number of samples collected so far. This quickly maxes out
   /// and remains equal to N.
   size_t sample_count;

   /// A ring buffer of the most recent sample_count raw ADC sample
   /// values.
   float sample[N];

   /// The number of values in each bin of a histogram.
   float histogram[K];

   /// The sum of the values in each bin of a histogram.
   float binsum[K];

   /// The most recent voltage reading.
   int32_t mv_hold;
};
