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
 * Module: EduBtM_Fetch.c
 *
 * Description :
 *  Find the first object satisfying the given condition.
 *  If there is no such object, then return with 'flag' field of cursor set
 *  to CURSOR_EOS. If there is an object satisfying the condition, then cursor
 *  points to the object position in the B+ tree and the object identifier
 *  is returned via 'cursor' parameter.
 *  The condition is given with a key value and a comparison operator;
 *  the comparison operator is one among SM_BOF, SM_EOF, SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Exports:
 *  Four EduBtM_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*);



/*@================================
 * EduBtM_Fetch()
 *================================*/
/*
 * Function: Four EduBtM_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition. See above for detail.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  cursor  : The found ObjectID and its position in the Btree Leaf
 *            (it may indicate a ObjectID in an  overflow page).
 */
Four EduBtM_Fetch(
    PageID   *root,		/* IN The current root of the subtree */
    KeyDesc  *kdesc,		/* IN Btree key descriptor */
    KeyValue *startKval,	/* IN key value of start condition */
    Four     startCompOp,	/* IN comparison operator of start condition */
    KeyValue *stopKval,		/* IN key value of stop condition */
    Four     stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor *cursor)	/* OUT Btree Cursor */
{
    int i;
    Four e;		   /* error number */

    
    if (root == NULL) ERR(eBADPARAMETER_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree index에서 검색 조건을 만족하는 object를 찾아 cursor를 반환한다.

    // 1) startCompOp가 SM_BOF일 경우: 가장 작은 object를 찾아 반환한다.
    if (startCompOp == SM_BOF) e = edubtm_FirstObject(root, kdesc, stopKval, stopCompOp, cursor);
    // 2) startCompOp가 SM_EOF일 경우: 가장 큰 object를 찾아 반환한다.
    else if (startCompOp == SM_EOF) e = edubtm_LastObject(root, kdesc, stopKval, stopCompOp, cursor);
    // 3) 이외의 경우, edubtm_fetch()를 호출해 검색 조건을 만족하는 첫 <object key, object ID>를 찾아 leaf index entry를 반환한다.
    else e = edubtm_Fetch(root, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
    
    // 디버깅용
    if (e < 0) ERR(e);
    return(eNOERROR);

} /* EduBtM_Fetch() */



/*@================================
 * edubtm_Fetch()
 *================================*/
/*
 * Function: Four edubtm_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition.
 *  This function handles only the following conditions:
 *  SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Returns:
 *  Error code *   
 *    eBADCOMPOP_BTM
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 */
Four edubtm_Fetch(
    PageID              *root,          /* IN The current root of the subtree */
    KeyDesc             *kdesc,         /* IN Btree key descriptor */
    KeyValue            *startKval,     /* IN key value of start condition */
    Four                startCompOp,    /* IN comparison operator of start condition */
    KeyValue            *stopKval,      /* IN key value of stop condition */
    Four                stopCompOp,     /* IN comparison operator of stop condition */
    BtreeCursor         *cursor)        /* OUT Btree Cursor */
{
    Four                e;              /* error number */
    Four                cmp;            /* result of comparison */
    Two                 idx;            /* index */
    PageID              child;          /* child page when the root is an internla page */
    Two                 alignedKlen;    /* aligned size of the key length */
    BtreePage           *apage;         /* a Page Pointer to the given root */
    BtreeOverflow       *opage;         /* a page pointer if it necessary to access an overflow page */
    Boolean             found;          /* search result */
    PageID              *leafPid;       /* leaf page pointed by the cursor */
    Two                 slotNo;         /* slot pointed by the slot */
    PageID              ovPid;          /* PageID of the overflow page */
    PageNo              ovPageNo;       /* PageNo of the overflow page */
    PageID              prevPid;        /* PageID of the previous page */
    PageID              nextPid;        /* PageID of the next page */
    ObjectID            *oidArray;      /* array of the ObjectIDs */
    Two                 iEntryOffset;   /* starting offset of an internal entry */
    btm_InternalEntry   *iEntry;        /* an internal entry */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry       *lEntry;        /* a leaf entry */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // 파라미터로 주어진 page를 root page로 하는 B+ tree index에서 검색조건을 만족하는 첫 <object key, object ID>를 반환
    // (검색 시작 key 값과 가장 가까운 key 값)
    BfM_GetTrain(root, &apage, PAGE_BUF);

    // case 1. root page가 internal page
    if (apage->any.hdr.type & INTERNAL) {
        // 1) 방문해야할 child page를 찾는다.
        found = edubtm_BinarySearchInternal(&apage->bi, kdesc, startKval, &idx);
        if (idx < 0) MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
        else {
            iEntry = &(apage->bi.data[apage->bi.slot[-idx]]);
            MAKE_PAGEID(child, root->volNo, iEntry->spid);
        }
        // 2) 재귀적으로 edubtm_Fetch()를 불러, leaf index entry를 탐색한다.
        edubtm_Fetch(&child, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
        BfM_FreeTrain(root, PAGE_BUF);
    }
    // case 2. root page가 leaf page
    else if (apage->any.hdr.type & LEAF) {
        // 1) 검색 조건을 만족하는 첫 <object key, object ID>가 저장된 index entry를 검색함.
        // (SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE 5가지 경우를 처리해주어야함)
        leafPid = root;
        cursor->flag = CURSOR_ON;

        found = edubtm_BinarySearchLeaf(&apage->bl, kdesc, startKval, &slotNo);
        // For SM_EOF, SM_BOF
        idx = slotNo;

        // case 1. SM_EQ 
        if (startCompOp == SM_EQ) {
            if (found == TRUE) idx = slotNo;
            else {
                cursor->flag = CURSOR_EOS;
                BfM_FreeTrain(root, PAGE_BUF);
                return eNOERROR;
            }
        }
        // case 2. SM_LT
        if (startCompOp == SM_LT) {
            if (found == TRUE) idx = slotNo - 1;
            else idx = slotNo;
        }
        // case 3. SM_LE
        if (startCompOp == SM_LE) {
            if (found == TRUE) idx = slotNo;
            else idx = slotNo;
        }
        // case 4. SM_GE
        if (startCompOp == SM_GE) {
            if (found == TRUE) idx = slotNo;
            else idx = slotNo + 1;
        }
        // case 5. SM_GT
        if (startCompOp == SM_GT) {
            if (found == TRUE) idx = slotNo + 1;
            else idx = slotNo + 1;
        }

        // 다음/이전 page에서 원소를 가져와야하는지를 검사
        // case 1. 이전 page에서 마지막 원소를 가지고 와야함
        if (idx < 0) {
            if (apage->bl.hdr.prevPage == NIL) cursor->flag = CURSOR_EOS;
            else {
                MAKE_PAGEID(prevPid, root->volNo, apage->bl.hdr.prevPage);
                leafPid = &prevPid;

                BfM_FreeTrain(root, PAGE_BUF);
                BfM_GetTrain(leafPid, &apage, PAGE_BUF);
                idx = apage->bl.hdr.nSlots - 1;
            }
        }
        // case 2. 다음 page에서 첫 원소를 가지고 와야함
        else if (idx >= apage->bl.hdr.nSlots) {
            if (apage->bl.hdr.nextPage == NIL) cursor->flag = CURSOR_EOS;
            else {
                MAKE_PAGEID(nextPid, root->volNo, apage->bl.hdr.nextPage);
                leafPid = &nextPid;

                BfM_FreeTrain(root, PAGE_BUF);
                BfM_GetTrain(leafPid, &apage, PAGE_BUF);
                idx = 0;
            }
        }

        cursor->leaf = *leafPid;
        cursor->slotNo = idx;

        lEntryOffset = apage->bl.slot[-idx];
        lEntry = &(apage->bl.data[lEntryOffset]);
        alignedKlen = ALIGNED_LENGTH(lEntry->klen);

        memcpy(&cursor->oid, &lEntry->kval + alignedKlen, sizeof(ObjectID));
        memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));

        cmp = edubtm_KeyCompare(kdesc, &cursor->key, stopKval);
        if (stopCompOp == SM_LE && (cmp != LESS && cmp != EQUAL)) cursor->flag = CURSOR_EOS;
        else if (stopCompOp == SM_LT && cmp != LESS) cursor->flag = CURSOR_EOS;
        else if (stopCompOp == SM_GE && (cmp != GREAT && cmp != EQUAL)) cursor->flag = CURSOR_EOS;
        else if (stopCompOp == SM_GT && cmp != GREAT) cursor->flag = CURSOR_EOS;

        BfM_FreeTrain(leafPid, PAGE_BUF);
    }

    return(eNOERROR);
} /* edubtm_Fetch() */

