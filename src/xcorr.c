#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include "../include/xcorr.h"


void xcorr_fftw(void *signala, void *signalb, void *result, int N)
{
#ifdef FFTW_ENABLED
#include <fftw3.h>
	int N2 = 2 * N - 1;
	complex * signala_ext = (complex *) calloc(N2, sizeof(complex));
	complex * signalb_ext = (complex *) calloc(N2, sizeof(complex));
	// zero padding
	memcpy(signala_ext + (N - 1), signala, sizeof(complex) * N);
	memcpy(signalb_ext, signalb, sizeof(complex) * N);
	/* Have FFTW, using FFT cross-correlation */
	fftw_complex * outa = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N2);
	fftw_complex * outb = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N2);
	fftw_complex * out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N2);

	fftw_plan pa = fftw_plan_dft_1d(N2, signala_ext, outa, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_plan pb = fftw_plan_dft_1d(N2, signalb_ext, outb, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_plan px = fftw_plan_dft_1d(N2, out, result, FFTW_BACKWARD, FFTW_ESTIMATE);

	fftw_execute(pa);
	fftw_execute(pb);

	fftw_complex scale = 1.0/(2 * N -1);
	for (int i = 0; i < N2; ++i)
		out[i] = outa[i] * conj(outb[i]) * scale;

	fftw_execute(px);

	fftw_destroy_plan(pa);
	fftw_destroy_plan(pb);
	fftw_destroy_plan(px);

	fftw_free(out);
	fftw_free(outa);
	fftw_free(outb);

	fftw_cleanup();

	free(signala_ext);
	free(signalb_ext);
	return;
#else
	fprintf(stderr, "No FFTW, use 'xcorr' or 'xcorr_timedomain'\n");
	exit(-1);
#endif /* FFTW_ENABLED */
}

void xcorr_timedomain(void *signala, void *signalb, void *result, int N)
{
	int N2 = 2 * N - 1;
	complex * signala_ext = (complex *) calloc(N2, sizeof(complex));
	complex * signalb_ext = (complex *) calloc(N2, sizeof(complex));
	// zero padding
	memcpy(signala_ext + (N - 1), signala, sizeof(complex) * N);
	memcpy(signalb_ext, signalb, sizeof(complex) * N);
	/* No FFTW, using time-domain cross-correlation */
	complex acf;
	for (int tau = 0; tau < N2; ++tau) {
		acf = 0 + 0*I;
		for (int i = 0; i < N2; ++i) {
			acf += signala_ext[(i+tau) % N2] * conj(signalb_ext[i % N2]);
		}
		memcpy((complex*) result + tau, &acf, sizeof(complex));
	}
	free(signala_ext);
	free(signalb_ext);
	return;
}

void xcorr(void *signala, void *signalb, void *result, int N)
{
#ifdef FFTW_ENABLED
	return xcorr_fftw(signala, signalb, result, N);
#else
	return xcorr_timedomain(signala, signalb, result, N);
#endif /* FFTW_ENABLED */
}


