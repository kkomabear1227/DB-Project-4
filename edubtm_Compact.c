/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubtm_Compact.c
 * 
 * Description:
 *  Two functions edubtm_CompactInternalPage() and edubtm_CompactLeafPage() are
 *  used to compact the internal page and the leaf page, respectively.
 *
 * Exports:
 *  void edubtm_CompactInternalPage(BtreeInternal*, Two)
 *  void edubtm_CompactLeafPage(BtreeLeaf*, Two)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_CompactInternalPage()
 *================================*/
/*
 * Function: edubtm_CompactInternalPage(BtreeInternal*, Two)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Reorganize the internal page to make sure the unused bytes in the page
 *  are located contiguously "in the middle", between the entries and the
 *  slot array. To compress out holes, entries must be moved toward the
 *  beginning of the page.
 *
 * Returns:
 *  None
 *
 * Side effects:
 *  The leaf page is reorganized to compact the space.
 */
void edubtm_CompactInternalPage(
    BtreeInternal       *apage,                 /* INOUT internal page to compact */
    Two                 slotNo)                 /* IN slot to go to the boundary of free space */
{
    BtreeInternal       tpage;                  /* temporay page used to save the given page */
    Two                 apageDataOffset;        /* where the next object is to be moved */
    Two                 len;                    /* length of the leaf entry */
    Two                 i;                      /* index variable */
    btm_InternalEntry   *entry;                 /* an entry in leaf page */

    // 1) Internal page의 모든 free space가 하나의 연속된 공간을 만들도록 offset을 재조정
    apageDataOffset = 0;

    // Case 1. 만약 slotNo가 NIL이 아니라면
    if (slotNo != NIL) {
        // slotNo에 대응하는 index entry를 제외한 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                
                memcpy(tpage.data + apageDataOffset, entry, len);
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += len;
            }
        }
        // slotNo에 대응되는 index entry는 마지막 index entry로 저장한다.
        entry = apage->data + apage->slot[-slotNo];
        len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                
        memcpy(tpage.data + apageDataOffset, entry, len);
        apage->slot[-slotNo] = apageDataOffset;
        apageDataOffset += len;

        memcpy(apage->data, tpage.data, apageDataOffset);
    }
    // Case 2. 만약 slotNo가 NIL이라면
    else {
        // page의 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                
                memcpy(tpage.data + apageDataOffset, entry, len);
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += len;
            }
        }
        memcpy(apage->data, tpage.data, apageDataOffset);
    }

    // 2) Page header 갱신
    apage->hdr.free = apageDataOffset;
    apage->hdr.unused = 0;

} /* edubtm_CompactInternalPage() */



/*@================================
 * edubtm_CompactLeafPage()
 *================================*/
/*
 * Function: void edubtm_CompactLeafPage(BtreeLeaf*, Two)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Reorganizes the leaf page to make sure the unused bytes in the page
 *  are located contiguously "in the middle", between the entries and the
 *  slot array. To compress out holes, entries must be moved toward the
 *  beginning of the page.
 *	
 * Return Values :
 *  None
 *
 * Side Effects :
 *  The leaf page is reorganized to comact the space.
 */
void edubtm_CompactLeafPage(
    BtreeLeaf 		*apage,			/* INOUT leaf page to compact */
    Two       		slotNo)			/* IN slot to go to the boundary of free space */
{	
    BtreeLeaf 		tpage;			/* temporay page used to save the given page */
    Two                 apageDataOffset;        /* where the next object is to be moved */
    Two                 len;                    /* length of the leaf entry */
    Two                 i;                      /* index variable */
    btm_LeafEntry 	*entry;			/* an entry in leaf page */
    Two 		alignedKlen;		/* aligned length of the key length */

    // 1) Leaf page의 모든 free space가 하나의 연속된 공간을 만들도록 offset을 재조정
    apageDataOffset = 0;

    // Case 1. 만약 slotNo가 NIL이 아니라면
    if (slotNo != NIL) {
        // slotNo에 대응하는 index entry를 제외한 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                //len = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                // overflow?
                
                memcpy(tpage.data + apageDataOffset, entry, OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen));
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
            }
        }
        // slotNo에 대응되는 index entry는 마지막 index entry로 저장한다.
        entry = apage->data + apage->slot[-slotNo];
        //len = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
        // overflow?
                
        memcpy(tpage.data + apageDataOffset, entry, OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen));
        apage->slot[-slotNo] = apageDataOffset;
        apageDataOffset += OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);

        memcpy(apage->data, tpage.data, apageDataOffset);
    }
    // Case 2. 만약 slotNo가 NIL이라면
    else {
        // page의 모든 index entry를 앞에서부터 저장한다.
        for (i = 0; i < apage->hdr.nSlots; i++) {
            if (i != slotNo) {
                entry = apage->data + apage->slot[-i];
                //len = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
                // overflow?

                memcpy(tpage.data + apageDataOffset, entry, OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen));
                apage->slot[-i] = apageDataOffset;
                apageDataOffset += OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen);
            }
        }
        memcpy(apage->data, tpage.data, apageDataOffset);
    }

    // 2) Page header 갱신
    apage->hdr.free = apageDataOffset;
    apage->hdr.unused = 0;

} /* edubtm_CompactLeafPage() */
