#ifndef CONFIGS_SANITY_H_INCLUDED
#define CONFIGS_SANITY_H_INCLUDED

#ifndef CONFIGS_H_INCLUDED
#error Must include configs.h before including configs_sanity.h
#endif

/* BEGIN SANITY CHECKS FOR INVALID CONFIGURATION */
#if (defined(BLOCKID_INTO_MEM)) && (defined(BLOCKID_NOMEM_SEARCH))
#error Cannot define both BLOCKID_INTO_MEM and BLOCKID_NOMEM_SEARCH
#endif
/* END SANITY CHECKS FOR INVALID CONFIGURATION */

#endif
