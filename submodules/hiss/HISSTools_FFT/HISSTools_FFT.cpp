

#include "HISSTools_FFT.h"

// Internal

long SSE_Exists = 0;

// Compile FFT Basics for double and single precision

#include "FFT_Core.h"

#ifdef USE_APPLE_FFT

// User FFT Routines

// Complex 

void hisstools_fft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
	vDSP_fft_zipD(setup, input, (vDSP_Stride) 1, log2n, FFT_FORWARD);
}

void hisstools_fft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
	vDSP_fft_zip(setup, input, (vDSP_Stride) 1, log2n, FFT_FORWARD);
}

// Real

void hisstools_rfft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
	vDSP_fft_zripD(setup, input, (vDSP_Stride) 1, log2n, FFT_FORWARD);
}

void hisstools_rfft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
	vDSP_fft_zrip(setup, input, (vDSP_Stride) 1, log2n, FFT_FORWARD);
}

// Complex Inverse

void hisstools_ifft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
	vDSP_fft_zipD(setup, input, (vDSP_Stride) 1, log2n, FFT_INVERSE);
}

void hisstools_ifft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
	vDSP_fft_zip(setup, input, (vDSP_Stride) 1, log2n, FFT_INVERSE);
}

// Real Inverse

void hisstools_rifft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
	vDSP_fft_zripD(setup, input, (vDSP_Stride) 1, log2n, FFT_INVERSE);
}	

void hisstools_rifft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
	vDSP_fft_zrip(setup, input, (vDSP_Stride) 1, log2n, FFT_INVERSE);
}

#else

// User FFT Routines

// Complex

void hisstools_fft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
    hisstools_fft(input, setup, log2n);
}

void hisstools_fft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
    hisstools_fft(input, setup, log2n);
}

// Real

void hisstools_rfft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
    hisstools_rfft(input, setup, log2n);
}

void hisstools_rfft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
    hisstools_rfft(input, setup, log2n);
}

// Complex Inverse

void hisstools_ifft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
    hisstools_ifft(input, setup, log2n);
}

void hisstools_ifft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
    hisstools_ifft(input, setup, log2n);
}

// Real Inverse

void hisstools_rifft_d(FFT_SETUP_D setup, FFT_SPLIT_COMPLEX_D *input, uintptr_t log2n)
{
    hisstools_rifft(input, setup, log2n);
}

void hisstools_rifft_f(FFT_SETUP_F setup, FFT_SPLIT_COMPLEX_F *input, uintptr_t log2n)
{
    hisstools_rifft(input, setup, log2n);
}

#endif


// Unzip incorporating zero padding

void hisstools_unzip_zero_d(double *input, FFT_SPLIT_COMPLEX_D *output, uintptr_t in_length, uintptr_t log2n)
{
	uintptr_t i;
	double temp = 0;
	
	double *realp = output->realp;
	double *imagp = output->imagp;
	
	// Check input length and get last value if in_length is odd
	
	if (((uintptr_t) 1 << log2n) < in_length)
		in_length = (uintptr_t) 1 << log2n;
	if (in_length & 1)
		temp = input[in_length - 1];
	
	// Unzip an even number of samples
		
#ifdef USE_APPLE_FFT	
	vDSP_ctozD((DOUBLE_COMPLEX *) input, (vDSP_Stride) 2, output, (vDSP_Stride) 1, in_length >> 1);
#else
	unzipComplex (input, output, in_length >> 1);
#endif
	
	// If necessary replace the odd sample, and zero pad the input
		
	if (((uintptr_t) 1 << log2n) > in_length)
	{
		realp[in_length >> 1] = temp;
		imagp[in_length >> 1] = 0;
	
		for (i = (in_length >> (uintptr_t) 1) + 1; i < ((uintptr_t) 1 << (log2n - (uintptr_t) 1)); i++)
		{
			realp[i] = 0.;
			imagp[i] = 0.;
		}
	}
}

void hisstools_unzip_zero_f(float *input, FFT_SPLIT_COMPLEX_F *output, uintptr_t in_length, uintptr_t log2n)
{
	uintptr_t i;
	float temp = 0;
	
	float *realp = output->realp;
	float *imagp = output->imagp;
	
	// Check input length and get last value if in_length is odd
	
	if (((uintptr_t) 1 << log2n) < in_length)
		in_length = (uintptr_t) 1 << log2n;
	if (in_length & 1)
		temp = input[in_length - 1];
	
	// Unzip an even number of samples
		
    #ifdef USE_APPLE_FFT
	vDSP_ctoz((COMPLEX *) input, (vDSP_Stride) 2, output, (vDSP_Stride) 1, in_length >> 1);
#else
	unzipComplex(input, output, in_length >> 1);
#endif
		
	// If necessary replace the odd sample, and zero pad the input
		
	if (((uintptr_t) 1 << log2n) > in_length)
	{
		realp[in_length >> 1] = temp;
		imagp[in_length >> 1] = 0.f;
		
		for (i = (in_length >> (uintptr_t) 1) + 1; i < ((uintptr_t) 1 << (log2n - (uintptr_t) 1)); i++)
		{
			realp[i] = 0.f;
			imagp[i] = 0.f;
		}
	}
}

// N.B This routine specifically deals with unzipping float data into a double precision complex split format, thus removing the need for a temp memory location 

void hisstools_unzip_zero_fd (float *input, FFT_SPLIT_COMPLEX_D *output, uintptr_t in_length, uintptr_t log2n)
{
	uintptr_t i;
	float temp = 0;
	
	double *realp = output->realp;
	double *imagp = output->imagp;
	
	// Check input length and get last value if in_length is odd
	
	if (((uintptr_t) 1 << log2n) < in_length)
		in_length = (uintptr_t) 1 << log2n;
	if (in_length & 1)
		temp = input[in_length - 1];	
	
	// Unzip an even number of samples
			
	for (i = 0; i < (in_length >> 1); i++)
	{
		*realp++ = *input++;
		*imagp++ = *input++;
	}
	
	// If necessary replace the odd sample, and zero pad the input
	
	if (((uintptr_t) 1 << log2n) > in_length)
	{
		*realp++ = temp;
		*imagp++ = 0.;
		
		for (i = (in_length >> (uintptr_t) 1) + 1; i < ((uintptr_t) 1 << (log2n - (uintptr_t) 1)); i++)
		{
			*realp++ = 0.;
			*imagp++ = 0.;
		}
	}
}

// Zip and Unzip

#ifdef USE_APPLE_FFT

void hisstools_unzip_d(double *input, FFT_SPLIT_COMPLEX_D *output, uintptr_t log2n)
{
	vDSP_ctozD((DOUBLE_COMPLEX *) input, (vDSP_Stride) 2, output, (vDSP_Stride) 1, (vDSP_Length) (1 << (log2n - 1)));
}

void hisstools_unzip_f(float *input, FFT_SPLIT_COMPLEX_F *output, uintptr_t log2n)
{
	vDSP_ctoz((COMPLEX *) input, (vDSP_Stride) 2, output, (vDSP_Stride) 1, (vDSP_Length) (1 << (log2n - 1)));
}

void hisstools_zip_d(FFT_SPLIT_COMPLEX_D *input, double *output, uintptr_t log2n)
{
	vDSP_ztocD(input, (vDSP_Stride) 1, (DOUBLE_COMPLEX *) output, (vDSP_Stride) 2, (vDSP_Length) (1 << (log2n - 1)));
}

void hisstools_zip_f(FFT_SPLIT_COMPLEX_F *input, float *output, uintptr_t log2n)
{
	vDSP_ztoc(input, (vDSP_Stride) 1, (COMPLEX *) output, (vDSP_Stride) 2, (vDSP_Length) (1 << (log2n - 1)));
}

// Setup Create / Destory

FFT_SETUP_D hisstools_create_setup_d(uintptr_t max_fft_log_2)
{
	return vDSP_create_fftsetupD(max_fft_log_2, FFT_RADIX2);
}

FFT_SETUP_F hisstools_create_setup_f(uintptr_t max_fft_log_2)
{
	return vDSP_create_fftsetup(max_fft_log_2, FFT_RADIX2);
}

void hisstools_destroy_setup_d(FFT_SETUP_D setup)
{
	if (setup)
		vDSP_destroy_fftsetupD(setup);
}

void hisstools_destroy_setup_f(FFT_SETUP_F setup)
{
	if (setup)
        vDSP_destroy_fftsetup(setup);
}

#else

void hisstools_unzip_d(double *input, FFT_SPLIT_COMPLEX_D *output, uintptr_t log2n)
{
    unzipComplex(input, output, (uintptr_t) 1 << (log2n - (uintptr_t) 1));
}

void hisstools_unzip_f(float *input, FFT_SPLIT_COMPLEX_F *output, uintptr_t log2n)
{
    unzipComplex(input, output, (uintptr_t) 1 << (log2n - (uintptr_t) 1));
}

void hisstools_zip_d(FFT_SPLIT_COMPLEX_D *input, double *output, uintptr_t log2n)
{
    zipComplex(input, output, (uintptr_t) 1 << (log2n - (uintptr_t) 1));
}

void hisstools_zip_f(FFT_SPLIT_COMPLEX_F *input, float *output, uintptr_t log2n)
{
    zipComplex(input, output, (uintptr_t) 1 << (log2n - (uintptr_t) 1));
}

// Setup Create / Destory

FFT_SETUP_D hisstools_create_setup_d(uintptr_t max_fft_log_2)
{
    return createSetup<double>(max_fft_log_2);
}

FFT_SETUP_F hisstools_create_setup_f(uintptr_t max_fft_log_2)
{
    return createSetup<float>(max_fft_log_2);
}

void hisstools_destroy_setup_d(FFT_SETUP_D setup)
{
    destroySetup(setup);
}

void hisstools_destroy_setup_f(FFT_SETUP_F setup)
{
    destroySetup(setup);
}

#endif
