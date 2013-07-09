/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*  
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 * This file is automatically generated by buildiface 
 * DO NOT EDIT
 */
#include "mpi_fortimpl.h"


/* Begin MPI profiling block */
#if defined(USE_WEAK_SYMBOLS) && !defined(USE_ONLY_MPI_NAMES) 
#if defined(HAVE_MULTIPLE_PRAGMA_WEAK)
extern FORT_DLL_SPEC void FORT_CALL MPI_WIN_DETACH( MPI_Fint *, void*, MPI_Fint * );
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach__( MPI_Fint *, void*, MPI_Fint * );
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach( MPI_Fint *, void*, MPI_Fint * );
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach_( MPI_Fint *, void*, MPI_Fint * );

#if defined(F77_NAME_UPPER)
#pragma weak MPI_WIN_DETACH = PMPI_WIN_DETACH
#pragma weak mpi_win_detach__ = PMPI_WIN_DETACH
#pragma weak mpi_win_detach_ = PMPI_WIN_DETACH
#pragma weak mpi_win_detach = PMPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak MPI_WIN_DETACH = pmpi_win_detach__
#pragma weak mpi_win_detach__ = pmpi_win_detach__
#pragma weak mpi_win_detach_ = pmpi_win_detach__
#pragma weak mpi_win_detach = pmpi_win_detach__
#elif defined(F77_NAME_LOWER_USCORE)
#pragma weak MPI_WIN_DETACH = pmpi_win_detach_
#pragma weak mpi_win_detach__ = pmpi_win_detach_
#pragma weak mpi_win_detach_ = pmpi_win_detach_
#pragma weak mpi_win_detach = pmpi_win_detach_
#else
#pragma weak MPI_WIN_DETACH = pmpi_win_detach
#pragma weak mpi_win_detach__ = pmpi_win_detach
#pragma weak mpi_win_detach_ = pmpi_win_detach
#pragma weak mpi_win_detach = pmpi_win_detach
#endif



#elif defined(HAVE_PRAGMA_WEAK)

#if defined(F77_NAME_UPPER)
extern FORT_DLL_SPEC void FORT_CALL MPI_WIN_DETACH( MPI_Fint *, void*, MPI_Fint * );

#pragma weak MPI_WIN_DETACH = PMPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach__( MPI_Fint *, void*, MPI_Fint * );

#pragma weak mpi_win_detach__ = pmpi_win_detach__
#elif !defined(F77_NAME_LOWER_USCORE)
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach( MPI_Fint *, void*, MPI_Fint * );

#pragma weak mpi_win_detach = pmpi_win_detach
#else
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach_( MPI_Fint *, void*, MPI_Fint * );

#pragma weak mpi_win_detach_ = pmpi_win_detach_
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(F77_NAME_UPPER)
#pragma _HP_SECONDARY_DEF PMPI_WIN_DETACH  MPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_detach__  mpi_win_detach__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_detach  mpi_win_detach
#else
#pragma _HP_SECONDARY_DEF pmpi_win_detach_  mpi_win_detach_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(F77_NAME_UPPER)
#pragma _CRI duplicate MPI_WIN_DETACH as PMPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _CRI duplicate mpi_win_detach__ as pmpi_win_detach__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _CRI duplicate mpi_win_detach as pmpi_win_detach
#else
#pragma _CRI duplicate mpi_win_detach_ as pmpi_win_detach_
#endif
#endif /* HAVE_PRAGMA_WEAK */
#endif /* USE_WEAK_SYMBOLS */
/* End MPI profiling block */


/* These definitions are used only for generating the Fortran wrappers */
#if defined(USE_WEAK_SYMBOLS) && defined(HAVE_MULTIPLE_PRAGMA_WEAK) && \
    defined(USE_ONLY_MPI_NAMES)
extern FORT_DLL_SPEC void FORT_CALL MPI_WIN_DETACH( MPI_Fint *, void*, MPI_Fint * );
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach__( MPI_Fint *, void*, MPI_Fint * );
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach( MPI_Fint *, void*, MPI_Fint * );
extern FORT_DLL_SPEC void FORT_CALL mpi_win_detach_( MPI_Fint *, void*, MPI_Fint * );

#if defined(F77_NAME_UPPER)
#pragma weak mpi_win_detach__ = MPI_WIN_DETACH
#pragma weak mpi_win_detach_ = MPI_WIN_DETACH
#pragma weak mpi_win_detach = MPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak MPI_WIN_DETACH = mpi_win_detach__
#pragma weak mpi_win_detach_ = mpi_win_detach__
#pragma weak mpi_win_detach = mpi_win_detach__
#elif defined(F77_NAME_LOWER_USCORE)
#pragma weak MPI_WIN_DETACH = mpi_win_detach_
#pragma weak mpi_win_detach__ = mpi_win_detach_
#pragma weak mpi_win_detach = mpi_win_detach_
#else
#pragma weak MPI_WIN_DETACH = mpi_win_detach
#pragma weak mpi_win_detach__ = mpi_win_detach
#pragma weak mpi_win_detach_ = mpi_win_detach
#endif

#endif

/* Map the name to the correct form */
#ifndef MPICH_MPI_FROM_PMPI
#if defined(USE_WEAK_SYMBOLS) && defined(HAVE_MULTIPLE_PRAGMA_WEAK)
/* Define the weak versions of the PMPI routine*/
#ifndef F77_NAME_UPPER
extern FORT_DLL_SPEC void FORT_CALL PMPI_WIN_DETACH( MPI_Fint *, void*, MPI_Fint * );
#endif
#ifndef F77_NAME_LOWER_2USCORE
extern FORT_DLL_SPEC void FORT_CALL pmpi_win_detach__( MPI_Fint *, void*, MPI_Fint * );
#endif
#ifndef F77_NAME_LOWER_USCORE
extern FORT_DLL_SPEC void FORT_CALL pmpi_win_detach_( MPI_Fint *, void*, MPI_Fint * );
#endif
#ifndef F77_NAME_LOWER
extern FORT_DLL_SPEC void FORT_CALL pmpi_win_detach( MPI_Fint *, void*, MPI_Fint * );

#endif

#if defined(F77_NAME_UPPER)
#pragma weak pmpi_win_detach__ = PMPI_WIN_DETACH
#pragma weak pmpi_win_detach_ = PMPI_WIN_DETACH
#pragma weak pmpi_win_detach = PMPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak PMPI_WIN_DETACH = pmpi_win_detach__
#pragma weak pmpi_win_detach_ = pmpi_win_detach__
#pragma weak pmpi_win_detach = pmpi_win_detach__
#elif defined(F77_NAME_LOWER_USCORE)
#pragma weak PMPI_WIN_DETACH = pmpi_win_detach_
#pragma weak pmpi_win_detach__ = pmpi_win_detach_
#pragma weak pmpi_win_detach = pmpi_win_detach_
#else
#pragma weak PMPI_WIN_DETACH = pmpi_win_detach
#pragma weak pmpi_win_detach__ = pmpi_win_detach
#pragma weak pmpi_win_detach_ = pmpi_win_detach
#endif /* Test on name mapping */
#endif /* Use multiple pragma weak */

#ifdef F77_NAME_UPPER
#define mpi_win_detach_ PMPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_win_detach_ pmpi_win_detach__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_win_detach_ pmpi_win_detach
#else
#define mpi_win_detach_ pmpi_win_detach_
#endif /* Test on name mapping */

/* This defines the routine that we call, which must be the PMPI version
   since we're renaming the Fortran entry as the pmpi version.  The MPI name
   must be undefined first to prevent any conflicts with previous renamings. */
#undef MPI_Win_detach
#define MPI_Win_detach PMPI_Win_detach 

#else

#ifdef F77_NAME_UPPER
#define mpi_win_detach_ MPI_WIN_DETACH
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_win_detach_ mpi_win_detach__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_win_detach_ mpi_win_detach
/* Else leave name alone */
#endif


#endif /* MPICH_MPI_FROM_PMPI */

/* Prototypes for the Fortran interfaces */
#include "fproto.h"
FORT_DLL_SPEC void FORT_CALL mpi_win_detach_ ( MPI_Fint *v1, void*v2, MPI_Fint *ierr ){
    *ierr = MPI_Win_detach( (MPI_Win)*v1, v2 );
}