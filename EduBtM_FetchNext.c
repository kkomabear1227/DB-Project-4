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
 * Module: EduBtM_FetchNext.c
 *
 * Description:
 *  Find the next ObjectID satisfying the given condition. The current ObjectID
 *  is specified by the 'current'.
 *
 * Exports:
 *  Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_FetchNext(KeyDesc*, KeyValue*, Four, BtreeCursor*, BtreeCursor*);



/*@================================
 * EduBtM_FetchNext()
 *================================*/
/*
 * Function: Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*,
 *                              Four, BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Fetch the next ObjectID satisfying the given condition.
 * By the B+ tree structure modification resulted from the splitting or merging
 * the current cursor may point to the invalid position. So we should adjust
 * the B+ tree cursor before using the cursor.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    eBADCURSOR
 *    some errors caused by function calls
 */
Four EduBtM_FetchNext(
    PageID                      *root,          /* IN root page's PageID */
    KeyDesc                     *kdesc,         /* IN key descriptor */
    KeyValue                    *kval,          /* IN key value of stop condition */
    Four                        compOp,         /* IN comparison operator of stop condition */
    BtreeCursor                 *current,       /* IN current B+ tree cursor */
    BtreeCursor                 *next)          /* OUT next B+ tree cursor */
{
    int							i;
    Four                        e;              /* error number */
    Four                        cmp;            /* comparison result */
    Two                         slotNo;         /* slot no. of a leaf page */
    Two                         oidArrayElemNo; /* element no. of the array of ObjectIDs */
    Two                         alignedKlen;    /* aligned length of key length */
    PageID                      overflow;       /* temporary PageID of an overflow page */
    Boolean                     found;          /* search result */
    ObjectID                    *oidArray;      /* array of ObjectIDs */
    BtreeLeaf                   *apage;         /* pointer to a buffer holding a leaf page */
    BtreeOverflow               *opage;         /* pointer to a buffer holding an overflow page */
    btm_LeafEntry               *entry;         /* pointer to a leaf entry */
    BtreeCursor                 tCursor;        /* a temporary Btree cursor */
  
    
    /*@ check parameter */
    if (root == NULL || kdesc == NULL || kval == NULL || current == NULL || next == NULL)
	ERR(eBADPARAMETER_BTM);
    
    /* Is the current cursor valid? */
    if (current->flag != CURSOR_ON && current->flag != CURSOR_EOS)
		ERR(eBADCURSOR);
    
    if (current->flag == CURSOR_EOS) return(eNOERROR);
    
    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree index에서 검색 조건을 만족하는 현재 object의 다음 object를 반환한다.
    edubtm_FetchNext(kdesc, kval, compOp, current, next);
    
    return(eNOERROR);
    
} /* EduBtM_FetchNext() */



/*@================================
 * edubtm_FetchNext()
 *================================*/
/*
 * Function: Four edubtm_FetchNext(KeyDesc*, KeyValue*, Four,
 *                              BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Get the next item. We assume that the current cursor is valid; that is.
 *  'current' rightly points to an existing ObjectID.
 *
 * Returns:
 *  Error code
 *    eBADCOMPOP_BTM
 *    some errors caused by function calls
 */
Four edubtm_FetchNext(
    KeyDesc  		*kdesc,		/* IN key descriptor */
    KeyValue 		*kval,		/* IN key value of stop condition */
    Four     		compOp,		/* IN comparison operator of stop condition */
    BtreeCursor 	*current,	/* IN current cursor */
    BtreeCursor 	*next)		/* OUT next cursor */
{
    Four 		e;		/* error number */
    Four 		cmp;		/* comparison result */
    Two 		alignedKlen;	/* aligned length of a key length */
    PageID 		leaf;		/* temporary PageID of a leaf page */
    PageID 		overflow;	/* temporary PageID of an overflow page */
    ObjectID 		*oidArray;	/* array of ObjectIDs */
    BtreeLeaf 		*apage;		/* pointer to a buffer holding a leaf page */
    BtreeOverflow 	*opage;		/* pointer to a buffer holding an overflow page */
    btm_LeafEntry 	*entry;		/* pointer to a leaf entry */    
    
    
    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree index에서 검색 조건을 만족하는 현재 object의 다음 object를 반환한다.
    // 1. 위치해있는 page와 slotNo를 알아낸다
    leaf = current->leaf;
    BfM_GetTrain(&leaf, &apage, PAGE_BUF);

    overflow = leaf;
    next->flag = CURSOR_ON;
    
    // 1) SM_EQ
    if (compOp == SM_EQ) {
        next->flag = CURSOR_EOS;
        BfM_FreeTrain(&leaf, PAGE_BUF);
        return (eNOERROR);
    }
    // 2) SM_LT, SM_LE (+ SM_EOF)
    // 더 작은 entry를 반환한다.
    else if (compOp == SM_LT || compOp == SM_LE || compOp == SM_EOF) {
        next->slotNo = current->slotNo + 1;
    }
    // 3) SM_GT, SM_GE (+ SM_BOF)
    // 더 큰 entry를 반환한다.
    else if (compOp == SM_GE || compOp == SM_GT || compOp == SM_BOF) {
        next->slotNo = current->slotNo - 1;
    }

    if (next->slotNo < 0) {
        if (apage->hdr.prevPage == NIL) next->flag = CURSOR_EOS;
        else {
            BfM_FreeTrain(&leaf, PAGE_BUF);

            MAKE_PAGEID(overflow, leaf.volNo, apage->hdr.prevPage);
            BfM_GetTrain(&overflow, &apage, PAGE_BUF);

            next->slotNo = apage->hdr.nSlots - 1;
        }
    }
    else if (next->slotNo >= apage->hdr.nSlots) {
        if (apage->hdr.nextPage == NIL) next->flag = CURSOR_EOS;
        else {
            BfM_FreeTrain(&leaf, PAGE_BUF);

            MAKE_PAGEID(overflow, leaf.volNo, apage->hdr.nextPage);
            BfM_GetTrain(&overflow, &apage, PAGE_BUF);

            next->slotNo = 0;
        }
    }
    else {
        BfM_FreeTrain(&leaf, PAGE_BUF);
        BfM_GetTrain(&overflow, &apage, PAGE_BUF);
    }
        
    // 2. 해당 정보들을 바탕으로, next entry의 남은 정보들을 기록한다.
    next->leaf = overflow;

    entry = &apage->data[apage->slot[-next->slotNo]];
	alignedKlen = ALIGNED_LENGTH(entry->klen);

	memcpy(&next->oid, &(entry->kval[alignedKlen]), OBJECTID_SIZE);
	memcpy(&next->key, &entry->klen, sizeof(KeyValue));

    cmp = edubtm_KeyCompare(kdesc, &next->key, kval);
	if (compOp == SM_LE && (cmp != LESS && cmp != EQUAL)) next->flag = CURSOR_EOS;
    else if (compOp == SM_LT && cmp != LESS) next->flag = CURSOR_EOS;
    else if (compOp == SM_GE && (cmp != GREAT && cmp != EQUAL)) next->flag = CURSOR_EOS;
    else if (compOp == SM_GT && (cmp != GREAT)) next->flag = CURSOR_EOS;
            
    // 4. 마무리
    BfM_FreeTrain(&overflow, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_FetchNext() */
