/************************************************************************/
/* Public implementation follows										*/
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
/*--- Defines ----------------------------------------------------------*/

#undef				SVDiterations
#define				SVDiterations	((int)30)

/*--- Types ------------------------------------------------------------*/
/* None */

/*--- Functions --------------------------------------------------------*/

extern	int			svdcmp			(double				*a,
									 long				m,
									 long				n,
									 double				w[],
									 double				*v);
extern	int			svbksb			(double				*u,
									 double				w[],
									 double				*v,
									 long				m,
									 long				n,
									 double				b[],
									 double				x[]);

/************************************************************************/
/* Externals															*/
/*----------------------------------------------------------------------*/
/* None																	*/
/************************************************************************/

#ifdef __cplusplus
}
#endif
