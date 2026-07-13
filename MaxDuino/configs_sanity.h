#ifndef CONFIGS_SANITY_H_INCLUDED
#define CONFIGS_SANITY_H_INCLUDED

#ifndef CONFIGS_H_INCLUDED
#error Must include configs.h before including configs_sanity.h
#endif

/* BEGIN SANITY CHECKS FOR INVALID CONFIGURATION */
#if (defined(BLOCKID_INTO_MEM)) && (defined(BLOCKID_NOMEM_SEARCH))
#error Cannot define both BLOCKID_INTO_MEM and BLOCKID_NOMEM_SEARCH
#endif

#if defined(DoubleFont) && !defined(XY2)
#error DoubleFont can only be used with XY2 , not XY
#endif


#ifdef RECORD

#if defined(RECORD_CAS_MSX) && !defined(Use_CAS)
#error RECORD_CAS_MSX requires Use_CAS
#endif

#endif // RECORD

/* END SANITY CHECKS FOR INVALID CONFIGURATION */


#endif
