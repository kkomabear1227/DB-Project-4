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
 * Module: edubtm_Delete.c
 *
 * Description : 
 *  This function edubtm_Delete(...) recursively calls itself until the type
 *  of root page becomes LEAF.  If the root page is an internal page, it
 *  may get the proper child page using the binary search routine and then
 *  recursively calls itself using the child as a root page. If the filled
 *  area of the child page is less than half of the page, it should merge
 *  or redistribute using the given root, and set the flag 'f' according to
 *  the result status of the given root page.
 *
 *  If the root page is a leaf page , it find out the correct node (entry)
 *  using the binary search routine.  If the entry is normal,  it simply
 *  delete the ObjectID or the entry when the # of ObjectIDs becomes zero.
 *  The entry, however, is not normal, that is, if the overflow page is used,
 *  the special routine btm_DeleteOverflow(...) should be called. The # of
 *  ObjectIDs will be returned by the result of the btm_DeleteOverflow(...),
 *  if the total # of ObjectIDs is less than 1/4 of the page and the ObjectIDs
 *  in the overflow page should be moved to the leaf page. (This process may
 *  has a complicate problem which the leaf page may be splitted in spite of
 *  deleteing not inserting an ObjectID.)
 *
 *  Deleting an ObjectID may cause redistribute pages and by this reason, the
 *  page may be splitted.
 *
 * Exports:
 *  Four edubtm_Delete(ObjectID*, PageID*, KeyDesc*, KeyValue*, ObjectID*,
 *                  Boolean*, Boolean*, InternalItem*, Pool*, DeallocListElem*)
 *
 */


#include <string.h>
#include "EduBtM_common.h"
#include "Util.h"
#include "BfM.h"
#include "OM_Internal.h"	/* for "SlottedPage" including catalog object */
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_DeleteLeaf(PhysicalFileID*, PageID*, BtreeLeaf*, KeyDesc*, KeyValue*, ObjectID*,
		    Boolean*, Boolean*, InternalItem*, Pool*, DeallocListElem*);



/*@================================
 * edubtm_Delete()
 *================================*/
/*
 * Function: Four edubtm_Delete(ObjectID*, PageID*, KeyDesc*, KeyValue*,
 *                           ObjectID*, Boolean*, Boolean*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *
 * Returns:
 *  error code
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  f    : TRUE if the given root page is not half full.
 *  h    : TRUE if the given page is splitted.
 *  item : The internal item to be inserted into the parent if 'h' is TRUE.
 */
Four edubtm_Delete(
    ObjectID                    *catObjForFile, /* IN catalog object of B+ tree file */
    PageID                      *root,          /* IN root page */
    KeyDesc                     *kdesc,         /* IN a key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN Object IDentifier which will be deleted */
    Boolean                     *f,             /* OUT whether the root page is half full */
    Boolean                     *h,             /* OUT TRUE if it is spiltted. */
    InternalItem                *item,          /* OUT The internal item to be returned */
    Pool                        *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem             *dlHead)        /* INOUT head of the dealloc list */
{
    Four                        e;              /* error number */
    Boolean                     lf;             /* TRUE if a page is not half full */
    Boolean                     lh;             /* TRUE if a page is splitted */
    Two                         idx;            /* the index by the binary search */
    PageID                      child;          /* a child page when the root is an internal page */
    KeyValue                    tKey;           /* a temporary key */
    BtreePage                   *rpage;         /* for a root page */
    InternalItem                litem;          /* local internal item */
    btm_InternalEntry           *iEntry;        /* an internal entry */
    SlottedPage                 *catPage;       /* buffer page containing the catalog object */
    sm_CatOverlayForBtree       *catEntry;      /* pointer to Btree file catalog information */
    PhysicalFileID              pFid;           /* B+-tree file's FileID */
  

    /* Error check whether using not supported functionality by EduBtM */
	int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
        
    *h = *f = FALSE;
    
    
    /* Delete following 2 lines before implement this function */
    //printf("and delete operation has not been implemented yet.\n");
    
    // 파라미터로 주어진 page를 root로 하는 B+ tree index에서 <object key, object ID>를 삭제함
    // 0) B+ tree를 얻어옴
    BfM_GetTrain(catObjForFile, &catPage, PAGE_BUF);
    GET_PTR_TO_CATENTRY_FOR_BTREE(catObjForFile, catPage, catEntry);
    MAKE_PAGEID(pFid, catEntry->fid.volNo, catEntry->firstPage);
    BfM_GetTrain(root, &rpage, PAGE_BUF);

    // Case 1. root page가 internal page
    if (rpage->any.hdr.type & INTERNAL) {
        // 1) 삭제할 pair가 저장된 leaf page를 찾기 위해 탐색을 진행
        edubtm_BinarySearchInternal(&rpage->bi, kdesc, kval, &idx);
        if (idx < 0) MAKE_PAGEID(child, root->volNo, rpage->bi.hdr.p0);
        else {
            iEntry = &rpage->bi.data[rpage->bi.slot[-idx]];
            MAKE_PAGEID(child, root->volNo, iEntry->spid);
        }

        // 2) B+ subtree에서 <object key, object ID>를 삭제하기 위해 재귀적으로 edubtm_Delete()를 호출
        edubtm_Delete(catObjForFile, &child, kdesc, kval, oid, &lf, &lh, &litem, dlPool, dlHead);

        // 3) underflow가 발생한 경우 btm_Unserflow()로 처리함.
        if (lf) {
            btm_Underflow(&pFid, rpage, &child, idx, f, &lh, &litem, dlPool, dlHead);
            BfM_SetDirty(root, PAGE_BUF);
            // a) underfloow가 발생한 자식 page의 부모 page에서 overflow가 발생한 경우,
            // edubtm_InsertInternal()을 호출한다.
            if (lh) {
                memcpy(&tKey, &litem.klen, sizeof(KeyValue));
                edubtm_BinarySearchInternal(rpage, kdesc, &tKey, &idx);
                edubtm_InsertInternal(catObjForFile, rpage, &litem, idx, h, item);
            }
        }
    }
    // Case 2. root page가 leaf page
    else if (rpage->any.hdr.type & LEAF) {
        // edubfm_DeleteLeaf()를 호출해 <object key, object ID>를 삭제함
        edubtm_DeleteLeaf(&pFid, root, rpage, kdesc, kval, oid, f, h, item, dlPool, dlHead);
    }

    BfM_FreeTrain(catObjForFile, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);

    return(eNOERROR);
    
}   /* edubtm_Delete() */



/*@================================
 * edubtm_DeleteLeaf()
 *================================*/
/*
 * Function: Four edubtm_DeleteLeaf(PhysicalFileID*, PageID*, BtreeLeaf*, KeyDesc*,
 *                               KeyValue*, ObjectID*, Boolean*, Boolean*,
 *                               InternalItem*, Pool*, DeallocListElem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *
 * Returns:
 *  Error code
 *    eNOTFOUND_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  f    : TRUE if the given root page is not half full.
 *  h    : TRUE if the given page is splitted.
 *  item : The internal item to be inserted into the parent if 'h' is TRUE.
 */ 
Four edubtm_DeleteLeaf(
    PhysicalFileID              *pFid,          /* IN FileID of the Btree file */
    PageID                      *pid,           /* IN PageID of the leaf page */
    BtreeLeaf                   *apage,         /* INOUT buffer for the Leaf Page */
    KeyDesc                     *kdesc,         /* IN a key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN ObjectID which will be deleted */
    Boolean                     *f,             /* OUT whether the root page is half full */
    Boolean                     *h,             /* OUT TRUE if it is spiltted. */
    InternalItem                *item,          /* OUT The internal item to be returned */
    Pool                        *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem             *dlHead)        /* INOUT head of a dealloc list */
{
    Four                        e;              /* error number */
    Two                         i;              /* index */
    Two                         of;             /* # of ObjectIDs of an overflow page when less than 1/4 */
    Two                         idx;            /* the index by the binary search */
    ObjectID                    tOid;           /* a Object IDentifier */
    BtreeOverflow               *opage;         /* for a overflow page */
    Boolean                     found;          /* Search Result */
    Two                         lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry               *lEntry;        /* an entry in leaf page */
    ObjectID                    *oidArray;      /* start position of the ObjectID array */
    Two                         oidArrayElemNo; /* element number in the ObjectIDs array */
    Two                         entryLen;       /* length of the old leaf entry */
    Two                         newLen;         /* length of the new leaf entry */
    Two                         alignedKlen;    /* aligned length of the key length */
    PageID                      ovPid;          /* overflow page's PageID */
    DeallocListElem             *dlElem;        /* an element of the dealloc list */


    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }


    /* Delete following 2 lines before implement this function */
    //printf("and delete operation has not been implemented yet.\n");

    // Leaf page에서 <object key, object ID> pair를 삭제한다.

    // 1) 삭제할 <object key, object ID> pair가 저장된 index entry의 slot을 삭제한다.
    // slot array는 compaction을 시킨다.
    found = edubtm_BinarySearchLeaf(apage, kdesc, kval, &idx);
    if (found) {
        lEntry = &apage->data[apage->slot[-idx]];
        lEntryOffset = apage->slot[-idx];
        
        entryLen = OBJECTID_SIZE + 2 * sizeof(Two) + ALIGNED_LENGTH(lEntry->klen);

        memcpy(&tOid, &lEntry->kval[ALIGNED_LENGTH(lEntry->klen)], OBJECTID_SIZE);

        // Compaction
        if (btm_ObjectIdComp(oid, &tOid) == EQUAL) {
            for (i = idx; i < apage->hdr.nSlots - 1; i++) {
                apage->slot[-i] = apage->slot[-(i+1)];
            }

            if (lEntryOffset + entryLen == apage->hdr.free) apage->hdr.free -= entryLen;
            else apage->hdr.unused += entryLen;

            apage->hdr.nSlots--;
        }
    }

    // 3) 만약 leaf page에서 underflow가 발생하면, f를 TRUE로 설정한다.
    if (BL_FREE(apage) > BL_HALF) *f = TRUE;

    BfM_SetDirty(pid, PAGE_BUF);
	      
    return(eNOERROR);
    
} /* edubtm_DeleteLeaf() */
