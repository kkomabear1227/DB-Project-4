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
 * Module: edubtm_Insert.c
 *
 * Description : 
 *  This function edubtm_Insert(...) recursively calls itself until the type
 *  of a root page becomes LEAF.  If the given root page is an internal,
 *  it recursively calls itself using a proper child.  If the result of
 *  the call occur spliting, merging, or redistributing the children, it
 *  may insert, delete, or replace its own internal item, and if the given
 *  root page may be merged, splitted, or redistributed, it affects the
 *  return values.
 *
 * Exports:
 *  Four edubtm_Insert(ObjectID*, PageID*, KeyDesc*, KeyValue*, ObjectID*,
 *                  Boolean*, Boolean*, InternalItem*, Pool*, DeallocListElem*)
 *  Four edubtm_InsertLeaf(ObjectID*, PageID*, BtreeLeaf*, KeyDesc*, KeyValue*,
 *                      ObjectID*, Boolean*, Boolean*, InternalItem*)
 *  Four edubtm_InsertInternal(ObjectID*, BtreeInternal*, InternalItem*,
 *                          Two, Boolean*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "OM_Internal.h"	/* for SlottedPage containing catalog object */
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_Insert()
 *================================*/
/*
 * Function: Four edubtm_Insert(ObjectID*, PageID*, KeyDesc*, KeyValue*,
 *                           ObjectID*, Boolean*, Boolean*, InternalItem*,
 *                           Pool*, DeallocListElem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  If the given root is a leaf page, it should get the correct entry in the
 *  leaf. If the entry is already in the leaf, it simply insert it into the
 *  entry and increment the number of ObjectIDs.  If it is not in the leaf it
 *  makes a new entry and insert it into the leaf.
 *  If there is not enough spage in the leaf, the page should be splitted.  The
 *  overflow page may be used or created by this routine. It is created when
 *  the size of the entry is greater than a third of a page.
 * 
 *  'h' is TRUE if the given root page is splitted and the entry item will be
 *  inserted into the parent page.  'f' is TRUE if the given page is not half
 *  full because of creating a new overflow page.
 *
 * Returns:
 *  Error code
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 */
Four edubtm_Insert(
    ObjectID                    *catObjForFile,         /* IN catalog object of B+-tree file */
    PageID                      *root,                  /* IN the root of a Btree */
    KeyDesc                     *kdesc,                 /* IN Btree key descriptor */
    KeyValue                    *kval,                  /* IN key value */
    ObjectID                    *oid,                   /* IN ObjectID which will be inserted */
    Boolean                     *f,                     /* OUT whether it is merged by creating a new overflow page */
    Boolean                     *h,                     /* OUT whether it is splitted */
    InternalItem                *item,                  /* OUT Internal Item which will be inserted */
                                                        /*     into its parent when 'h' is TRUE */
    Pool                        *dlPool,                /* INOUT pool of dealloc list */
    DeallocListElem             *dlHead)                /* INOUT head of the dealloc list */
{
    Four                        e;                      /* error number */
    Boolean                     lh;                     /* local 'h' */
    Boolean                     lf;                     /* local 'f' */
    Two                         idx;                    /* index for the given key value */
    PageID                      newPid;                 /* a new PageID */
    KeyValue                    tKey;                   /* a temporary key */
    InternalItem                litem;                  /* a local internal item */
    BtreePage                   *apage;                 /* a pointer to the root page */
    btm_InternalEntry           *iEntry;                /* an internal entry */
    Two                         iEntryOffset;           /* starting offset of an internal entry */
    SlottedPage                 *catPage;               /* buffer page containing the catalog object */
    sm_CatOverlayForBtree       *catEntry;              /* pointer to Btree file catalog information */
    PhysicalFileID              pFid;                   /* B+-tree file's FileID */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // 1. parameter로 주어진 page를 root page로 하는 B+ tree index에 대해
    // <object key, object ID>를 삽입하고, root split이 발생한 경우, 새 page를 가리키는 internal index entry를 반환

    *h = FALSE;
    *f = FALSE;

    BfM_GetTrain(root, &apage, PAGE_BUF);

    // Case 1. Root page가 INTERNAL page이다.
    if (apage->any.hdr.type & INTERNAL) {
        // 1) 새 <object key, object ID>를 삽입할 leaf page를 찾기 위해 binary search를 수행
        edubtm_BinarySearchInternal(apage, kdesc, kval, &idx);

        // 2) 결정된 자식 page를 root로 하는 B+ subtree에 새로운 <object key, object ID>를 삽입하기 위해
        //    edubtm_Insert()를 재귀적으로 호출
        //    만약 
        iEntryOffset = apage->bi.slot[-idx];
        iEntry = (btm_InternalEntry*) &apage->bi.data[iEntryOffset];

        if (idx < 0) MAKE_PAGEID(newPid, root->volNo, apage->bi.hdr.p0);
        else MAKE_PAGEID(newPid, root->volNo, iEntry->spid);

        lh = FALSE;
        lf = FALSE;

        edubtm_Insert(catObjForFile, &newPid, kdesc, kval, oid, &lf, &lh, &litem, dlPool, dlHead);
        // 3) 자식 page에서 split이 발생한 경우, split으로 생성된 . 새 page를 가리키는 index entry를 반환
        // 4) 파라미터로 주어진 root page에서 split이 발새앟면, split으로 생긴 새 page를 가리키는 index entry를 반환
        if (lh) {
            tKey.len = litem.klen;
            memcpy(tKey.val, litem.kval, tKey.len);
            edubtm_BinarySearchInternal(&(apage->bi), kdesc, &tKey, &idx);
            edubtm_InsertInternal(catObjForFile, &(apage->bi), &litem, idx, h, item);
        }
    }
    // Case 2. Root page가 LEAF page이다.
    else if (apage->any.hdr.type & LEAF) {
        //edubtm_InsertLeaf()를 호출한다. split이 일어나면 new page에 대한 internal index entry를 return
        edubtm_InsertLeaf(catObjForFile, root, &(apage->bl), kdesc, kval, oid, f, h, item);
    }

    BfM_SetDirty(root, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);
    
    return(eNOERROR);
    
}   /* edubtm_Insert() */



/*@================================
 * edubtm_InsertLeaf()
 *================================*/
/*
 * Function: Four edubtm_InsertLeaf(ObjectID*, PageID*, BtreeLeaf*, KeyDesc*,
 *                               KeyValue*, ObjectID*, Boolean*, Boolean*,
 *                               InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Insert into the given leaf page an ObjectID with the given key.
 *
 * Returns:
 *  Error code
 *    eDUPLICATEDKEY_BTM
 *    eDUPLICATEDOBJECTID_BTM
 *    some errors causd by function calls
 *
 * Side effects:
 *  1) f : TRUE if the leaf page is underflowed by creating an overflow page
 *  2) h : TRUE if the leaf page is splitted by inserting the given ObjectID
 *  3) item : item to be inserted into the parent
 */
Four edubtm_InsertLeaf(
    ObjectID                    *catObjForFile, /* IN catalog object of B+-tree file */
    PageID                      *pid,           /* IN PageID of Leag Page */
    BtreeLeaf                   *page,          /* INOUT pointer to buffer page of Leaf page */
    KeyDesc                     *kdesc,         /* IN Btree key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN ObjectID which will be inserted */
    Boolean                     *f,             /* OUT whether it is merged by creating */
                                                /*     a new overflow page */
    Boolean                     *h,             /* OUT whether it is splitted */
    InternalItem                *item)          /* OUT Internal Item which will be inserted */
                                                /*     into its parent when 'h' is TRUE */
{
    Four                        e;              /* error number */
    Two                         i;
    Two                         idx;            /* index for the given key value */
    LeafItem                    leaf;           /* a Leaf Item */
    Boolean                     found;          /* search result */
    btm_LeafEntry               *entry;         /* an entry in a leaf page */
    Two                         entryOffset;    /* start position of an entry */
    Two                         alignedKlen;    /* aligned length of the key length */
    PageID                      ovPid;          /* PageID of an overflow page */
    Two                         entryLen;       /* length of an entry */
    ObjectID                    *oidArray;      /* an array of ObjectIDs */
    Two                         oidArrayElemNo; /* an index for the ObjectID array */


    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    
    /*@ Initially the flags are FALSE */
    *h = *f = FALSE;
    
    // Leaf page에 새 index entry를 삽입한다.
    // split이 발생하면, split으로 생긴 새 leaf page를 가리키는 index entry를 반환한다.

    // 1) 새 index entry를 삽입하기 위한 위치를 결정한다.
    found = edubtm_BinarySearchLeaf(page, kdesc, kval, &idx);

    // 2) 새 index entry를 삽입하기 위해 필요한 free space를 계산
    if (kdesc->kpart[0].type == SM_INT) alignedKlen = ALIGNED_LENGTH(kdesc->kpart[0].length);
    else alignedKlen = ALIGNED_LENGTH(kval->len);

    entryLen = 2 * sizeof(Two) + alignedKlen + OBJECTID_SIZE;

    // index entry 설정
    leaf.oid = *oid;
    leaf.nObjects = 1;
    leaf.klen = kval->len;
    memcpy(leaf.kval, kval->val, leaf.klen);

    // Case 1. Page에 free space가 있다면
    if (BI_FREE(page) >= entryLen + sizeof(Two)) {
        // a) 필요시 page를 compaction함
        if (BI_CFREE(page) < entryLen + sizeof(Two)) edubtm_CompactLeafPage(page, NIL);
        // b) binary search로 찾은 위치에 index entry를 삽입함
        // b-1) 결정된 slotNo를 사용하기 위해 slot array를 재배열
        for (i = page->hdr.nSlots - 1; i >= 1 + idx; i--)
            page->slot[-(i+1)] = page->slot[-i];
        // 결정된 slotNo를 갖는 slot에 새 index entry의 offset을 저장
        entryOffset = page->slot[-(1 + idx)] = page->hdr.free;
        entry = (btm_LeafEntry*)&(page->data[entryOffset]);

        memcpy(entry, &leaf.nObjects, entryLen - OBJECTID_SIZE);
        memcpy(&entry->kval[alignedKlen], &leaf.oid, OBJECTID_SIZE);

        page->hdr.free += entryLen;
        page->hdr.nSlots++;
    }
    // Case 2. Page에 free space가 없다면
    else {
        // a) edubtm_SplitLeaf()을 호출해 page를 나눈다.
        // 새롭게 생긴 internal page를 가리키는 index entry를 반환한다.
        // index entry 설정
        edubtm_SplitLeaf(catObjForFile, pid, page, idx, &leaf, item);
        *h = TRUE;
    }

    return(eNOERROR);
    
} /* edubtm_InsertLeaf() */



/*@================================
 * edubtm_InsertInternal()
 *================================*/
/*
 * Function: Four edubtm_InsertInternal(ObjectID*, BtreeInternal*, InternalItem*, Two, Boolean*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  This routine insert the given internal item into the given page. If there
 *  is not enough space in the page, it should split the page and the new
 *  internal item should be returned for inserting into the parent.
 *
 * Returns:
 *  Error code
 *    some errors caused by function calls
 *
 * Side effects:
 *  h:	TRUE if the page is splitted
 *  ritem: an internal item which will be inserted into parent
 *          if spliting occurs.
 */
Four edubtm_InsertInternal(
    ObjectID            *catObjForFile, /* IN catalog object of B+-tree file */
    BtreeInternal       *page,          /* INOUT Page Pointer */
    InternalItem        *item,          /* IN Iternal item which is inserted */
    Two                 high,           /* IN index in the given page */
    Boolean             *h,             /* OUT whether the given page is splitted */
    InternalItem        *ritem)         /* OUT if the given page is splitted, the internal item may be returned by 'ritem'. */
{
    Four                e;              /* error number */
    Two                 i;              /* index */
    Two                 entryOffset;    /* starting offset of an internal entry */
    Two                 entryLen;       /* length of the new entry */
    btm_InternalEntry   *entry;         /* an internal entry of an internal page */


    
    /*@ Initially the flag are FALSE */
    *h = FALSE;
    
    // Internal page에 새 index entry를 삽입한다.
    // split이 발생하면, split으로 생긴 . 새internal page를 가리키는 index entry를 반환한다.

    // 1) 새 index entry를 삽입하기 위해 필요한 free space를 계산한다.
    entryLen = sizeof(ShortPageID) + sizeof(Two)+ ALIGNED_LENGTH(item->klen);

    // Case 1. Page에 free space가 있다면
    if (BI_FREE(page) >= entryLen + sizeof(Two)) {
        // a) 필요시 page를 compaction함
        if (BI_CFREE(page) < entryLen + sizeof(Two)) edubtm_CompactInternalPage(page, NIL);
        // b) 파라미터로 주어진 slotNo 다음에 index entry를 삽입
        for (i = page->hdr.nSlots - 1; i >= 1 + high; i--)
            page->slot[-(i+1)] = page->slot[-i];

        page->slot[-(high + 1)] = page->hdr.free;

        entry = page->data + page->hdr.free;
        entry->spid = item->spid;
        entry->klen = item->klen;
        memcpy(&entry->kval, item->kval, item->klen);

        page->hdr.free += entryLen;
        page->hdr.nSlots++;
    }
    // Case 2. Page에 free space가 없다면
    else {
        // a) edubtm_SplitInternal()을 호출해 page를 나눈다.
        // 새롭게 생긴 internal page를 가리키는 index entry를 반환한다.
        edubtm_SplitInternal(catObjForFile, page, high, item, ritem);
        *h = TRUE;
    }

    return(eNOERROR);
    
} /* edubtm_InsertInternal() */