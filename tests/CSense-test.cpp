// SPDX-License-Identifier: AGPL-3.0-or-later

#include <iostream>

using namespace std;

#include "VHP-Vibro-Glove2/CSense.hpp"

const double pi = 3.14159265359;

int main() {
  const int samplerate = 40000;
  const int volume = 450;

  CSense csense (samplerate,
		   0.0001f,
		   1.5f);

  for(int i=0; i < 150; i++) {
    float f = volume + volume * sin ( 2 * pi * i / 20);

    csense.record(0, f);

    cout << csense.get_samples(0) << "\t"
	 << f << "\t"
	 << csense.get_peak(0) << endl;
  }

  return 0;
}
