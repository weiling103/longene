/*
 * Unit test suite for crypt32.dll's CryptMsg functions
 *
 * Copyright 2007 Juan Lang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <winerror.h>
#include <wincrypt.h>

#include "wine/test.h"

static char oid_rsa_md5[] = szOID_RSA_MD5;

static BOOL (WINAPI * pCryptAcquireContextW)
                        (HCRYPTPROV *, LPCWSTR, LPCWSTR, DWORD, DWORD);

static void init_function_pointers(void)
{
    HMODULE hAdvapi32 = GetModuleHandleA("advapi32.dll");

#define GET_PROC(dll, func) \
    p ## func = (void *)GetProcAddress(dll, #func); \
    if(!p ## func) \
      trace("GetProcAddress(%s) failed\n", #func);

    GET_PROC(hAdvapi32, CryptAcquireContextW)

#undef GET_PROC
}

static void test_msg_open_to_encode(void)
{
    HCRYPTMSG msg;

    /* Crash
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_ENVELOPED, NULL,
     NULL, NULL);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, NULL, NULL,
     NULL);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, NULL, NULL,
     NULL);
     */

    /* Bad encodings */
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(0, 0, 0, NULL, NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(X509_ASN_ENCODING, 0, 0, NULL, NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());

    /* Bad message types */
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, 0, NULL, NULL, NULL);
    ok(!msg && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, 0,
     NULL, NULL, NULL);
    ok(!msg && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0,
     CMSG_SIGNED_AND_ENVELOPED, NULL, NULL, NULL);
    ok(!msg && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_ENCRYPTED, NULL,
     NULL, NULL);
    ok(!msg && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
}

static void test_msg_open_to_decode(void)
{
    HCRYPTMSG msg;
    CMSG_STREAM_INFO streamInfo = { 0 };

    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToDecode(0, 0, 0, 0, NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());

    /* Bad encodings */
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToDecode(X509_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToDecode(X509_ASN_ENCODING, 0, CMSG_DATA, 0, NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());

    /* The message type can be explicit... */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_ENVELOPED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0,
     CMSG_SIGNED_AND_ENVELOPED, 0, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    /* or implicit.. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    /* or even invalid. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_ENCRYPTED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 1000, 0, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    /* And even though the stream info parameter "must be set to NULL" for
     * CMSG_HASHED, it's still accepted.
     */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     &streamInfo);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
}

static void test_msg_get_param(void)
{
    BOOL ret;
    HCRYPTMSG msg;
    DWORD size, i, value;
    CMSG_SIGNED_ENCODE_INFO signInfo = { sizeof(signInfo), 0 };
    CMSG_SIGNER_ENCODE_INFO signer = { sizeof(signer), 0 };

    /* Crash
    ret = CryptMsgGetParam(NULL, 0, 0, NULL, NULL);
    ret = CryptMsgGetParam(NULL, 0, 0, NULL, &size);
    ret = CryptMsgGetParam(msg, 0, 0, NULL, NULL);
     */

    /* Decoded messages */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    /* For decoded messages, the type is always available */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    /* For this (empty) message, the type isn't set */
    ok(value == 0, "Expected type 0, got %d\n", value);
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    /* For explicitly typed messages, the type is known. */
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    ok(value == CMSG_DATA, "Expected CMSG_DATA, got %d\n", value);
    for (i = CMSG_CONTENT_PARAM; i <= CMSG_CMS_SIGNER_INFO_PARAM; i++)
    {
        size = 0;
        ret = CryptMsgGetParam(msg, i, 0, NULL, &size);
        ok(!ret, "Parameter %d: expected failure\n", i);
    }
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_ENVELOPED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    ok(value == CMSG_ENVELOPED, "Expected CMSG_ENVELOPED, got %d\n", value);
    for (i = CMSG_CONTENT_PARAM; i <= CMSG_CMS_SIGNER_INFO_PARAM; i++)
    {
        size = 0;
        ret = CryptMsgGetParam(msg, i, 0, NULL, &size);
        ok(!ret, "Parameter %d: expected failure\n", i);
    }
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    ok(value == CMSG_HASHED, "Expected CMSG_HASHED, got %d\n", value);
    for (i = CMSG_CONTENT_PARAM; i <= CMSG_CMS_SIGNER_INFO_PARAM; i++)
    {
        size = 0;
        ret = CryptMsgGetParam(msg, i, 0, NULL, &size);
        ok(!ret, "Parameter %d: expected failure\n", i);
    }
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    ok(value == CMSG_SIGNED, "Expected CMSG_SIGNED, got %d\n", value);
    for (i = CMSG_CONTENT_PARAM; i <= CMSG_CMS_SIGNER_INFO_PARAM; i++)
    {
        size = 0;
        ret = CryptMsgGetParam(msg, i, 0, NULL, &size);
        ok(!ret, "Parameter %d: expected failure\n", i);
    }
    CryptMsgClose(msg);

    /* Explicitly typed messages get their types set, even if they're invalid */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_ENCRYPTED, 0, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    ok(value == CMSG_ENCRYPTED, "Expected CMSG_ENCRYPTED, got %d\n", value);
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 1000, 0, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToDecode failed: %x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    ok(value == 1000, "Expected 1000, got %d\n", value);
    CryptMsgClose(msg);
}

static void test_msg_close(void)
{
    BOOL ret;
    HCRYPTMSG msg;

    /* NULL succeeds.. */
    ret = CryptMsgClose(NULL);
    ok(ret, "CryptMsgClose failed: %x\n", GetLastError());
    /* but an arbitrary pointer crashes. */
    if (0)
        ret = CryptMsgClose((HCRYPTMSG)1);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    ret = CryptMsgClose(msg);
    ok(ret, "CryptMsgClose failed: %x\n", GetLastError());
}

static void check_param(LPCSTR test, HCRYPTMSG msg, DWORD param,
 const BYTE *expected, DWORD expectedSize)
{
    DWORD size;
    LPBYTE buf;
    BOOL ret;

    size = 0xdeadbeef;
    ret = CryptMsgGetParam(msg, param, 0, NULL, &size);
    ok(ret, "%s: CryptMsgGetParam failed: %08x\n", test, GetLastError());
    buf = HeapAlloc(GetProcessHeap(), 0, size);
    ret = CryptMsgGetParam(msg, param, 0, buf, &size);
    ok(ret, "%s: CryptMsgGetParam failed: %08x\n", test, GetLastError());
    ok(size == expectedSize, "%s: expected size %d, got %d\n", test,
     expectedSize, size);
    if (size == expectedSize && size)
        ok(!memcmp(buf, expected, size), "%s: unexpected data\n", test);
    HeapFree(GetProcessHeap(), 0, buf);
}

static void test_data_msg_open(void)
{
    HCRYPTMSG msg;
    CMSG_HASHED_ENCODE_INFO hashInfo = { 0 };
    CMSG_STREAM_INFO streamInfo = { 0 };
    char oid[] = "1.2.3";

    /* The data message type takes no additional info */
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, &hashInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    /* An empty stream info is allowed. */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     &streamInfo);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Passing a bogus inner OID succeeds for a non-streamed message.. */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, oid,
     NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    /* and still succeeds when CMSG_DETACHED_FLAG is passed.. */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_DATA, NULL, oid, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    /* and when a stream info is given, even though you're not supposed to be
     * able to use anything but szOID_RSA_data when streaming is being used.
     */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_DATA, NULL, oid, &streamInfo);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
}

static const BYTE msgData[] = { 1, 2, 3, 4 };

static BOOL WINAPI nop_stream_output(const void *pvArg, BYTE *pb, DWORD cb,
 BOOL final)
{
    return TRUE;
}

static void test_data_msg_update(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    CMSG_STREAM_INFO streamInfo = { 0 };

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     NULL);
    /* Can't update a message that wasn't opened detached with final = FALSE */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    /* Updating it with final = TRUE succeeds */
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* Any subsequent update will fail, as the last was final */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     NULL);
    /* Can't update a message with no data */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    /* Curiously, a valid update will now fail as well, presumably because of
     * the last (invalid, but final) update.
     */
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_DATA, NULL, NULL, NULL);
    /* Doesn't appear to be able to update CMSG-DATA with non-final updates */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Calling update after opening with an empty stream info (with a bogus
     * output function) yields an error:
     */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     &streamInfo);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(!ret && GetLastError() == STATUS_ACCESS_VIOLATION,
     "Expected STATUS_ACCESS_VIOLATION, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* Calling update with a valid output function succeeds, even if the data
     * exceeds the size specified in the stream info.
     */
    streamInfo.pfnStreamOutput = nop_stream_output;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     &streamInfo);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);
}

static void test_data_msg_get_param(void)
{
    HCRYPTMSG msg;
    DWORD size;
    BOOL ret;
    CMSG_STREAM_INFO streamInfo = { 0, nop_stream_output, NULL };

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     NULL);

    /* Content and bare content are always gettable when not streaming */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_CONTENT_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_BARE_CONTENT_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    /* But for this type of message, the signer and hash aren't applicable,
     * and the type isn't available.
     */
    size = 0;
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_ENCODED_SIGNER, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Can't get content or bare content when streaming */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL,
     NULL, &streamInfo);
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_BARE_CONTENT_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_CONTENT_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    CryptMsgClose(msg);
}

static const BYTE dataEmptyBareContent[] = { 0x04,0x00 };
static const BYTE dataEmptyContent[] = {
0x30,0x0f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x02,
0x04,0x00 };
static const BYTE dataBareContent[] = { 0x04,0x04,0x01,0x02,0x03,0x04 };
static const BYTE dataContent[] = {
0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,
0x04,0x04,0x01,0x02,0x03,0x04 };

struct update_accum
{
    DWORD cUpdates;
    CRYPT_DATA_BLOB *updates;
};

static BOOL WINAPI accumulating_stream_output(const void *pvArg, BYTE *pb,
 DWORD cb, BOOL final)
{
    struct update_accum *accum = (struct update_accum *)pvArg;
    BOOL ret = FALSE;

    if (accum->cUpdates)
        accum->updates = CryptMemRealloc(accum->updates,
         (accum->cUpdates + 1) * sizeof(CRYPT_DATA_BLOB));
    else
        accum->updates = CryptMemAlloc(sizeof(CRYPT_DATA_BLOB));
    if (accum->updates)
    {
        CRYPT_DATA_BLOB *blob = &accum->updates[accum->cUpdates];

        blob->pbData = CryptMemAlloc(cb);
        if (blob->pbData)
        {
            memcpy(blob->pbData, pb, cb);
            blob->cbData = cb;
            ret = TRUE;
        }
        accum->cUpdates++;
    }
    return ret;
}

/* The updates of a (bogus) definite-length encoded message */
static BYTE u1[] = { 0x30,0x0f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
 0x07,0x01,0xa0,0x02,0x04,0x00 };
static BYTE u2[] = { 0x01,0x02,0x03,0x04 };
static CRYPT_DATA_BLOB b1[] = {
    { sizeof(u1), u1 },
    { sizeof(u2), u2 },
    { sizeof(u2), u2 },
};
static const struct update_accum a1 = { sizeof(b1) / sizeof(b1[0]), b1 };
/* The updates of a definite-length encoded message */
static BYTE u3[] = { 0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
 0x07,0x01,0xa0,0x06,0x04,0x04 };
static CRYPT_DATA_BLOB b2[] = {
    { sizeof(u3), u3 },
    { sizeof(u2), u2 },
};
static const struct update_accum a2 = { sizeof(b2) / sizeof(b2[0]), b2 };
/* The updates of an indefinite-length encoded message */
static BYTE u4[] = { 0x30,0x80,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
 0x07,0x01,0xa0,0x80,0x24,0x80 };
static BYTE u5[] = { 0x04,0x04 };
static BYTE u6[] = { 0x00,0x00,0x00,0x00,0x00,0x00 };
static CRYPT_DATA_BLOB b3[] = {
    { sizeof(u4), u4 },
    { sizeof(u5), u5 },
    { sizeof(u2), u2 },
    { sizeof(u5), u5 },
    { sizeof(u2), u2 },
    { sizeof(u6), u6 },
};
static const struct update_accum a3 = { sizeof(b3) / sizeof(b3[0]), b3 };

static void check_updates(LPCSTR header, const struct update_accum *expected,
 const struct update_accum *got)
{
    DWORD i;

    ok(expected->cUpdates == got->cUpdates,
     "%s: expected %d updates, got %d\n", header, expected->cUpdates,
     got->cUpdates);
    if (expected->cUpdates == got->cUpdates)
        for (i = 0; i < min(expected->cUpdates, got->cUpdates); i++)
        {
            ok(expected->updates[i].cbData == got->updates[i].cbData,
             "%s, update %d: expected %d bytes, got %d\n", header, i,
             expected->updates[i].cbData, got->updates[i].cbData);
            if (expected->updates[i].cbData && expected->updates[i].cbData ==
             got->updates[i].cbData)
                ok(!memcmp(expected->updates[i].pbData, got->updates[i].pbData,
                 got->updates[i].cbData), "%s, update %d: unexpected value\n",
                 header, i);
        }
}

/* Frees the updates stored in accum */
static void free_updates(struct update_accum *accum)
{
    DWORD i;

    for (i = 0; i < accum->cUpdates; i++)
        CryptMemFree(accum->updates[i].pbData);
    CryptMemFree(accum->updates);
    accum->updates = NULL;
    accum->cUpdates = 0;
}

static void test_data_msg_encoding(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    static char oid[] = "1.2.3";
    struct update_accum accum = { 0, NULL };
    CMSG_STREAM_INFO streamInfo = { 0, accumulating_stream_output, &accum };

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     NULL);
    check_param("data empty bare content", msg, CMSG_BARE_CONTENT_PARAM,
     dataEmptyBareContent, sizeof(dataEmptyBareContent));
    check_param("data empty content", msg, CMSG_CONTENT_PARAM, dataEmptyContent,
     sizeof(dataEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("data bare content", msg, CMSG_BARE_CONTENT_PARAM,
     dataBareContent, sizeof(dataBareContent));
    check_param("data content", msg, CMSG_CONTENT_PARAM, dataContent,
     sizeof(dataContent));
    CryptMsgClose(msg);
    /* Same test, but with CMSG_BARE_CONTENT_FLAG set */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_BARE_CONTENT_FLAG,
     CMSG_DATA, NULL, NULL, NULL);
    check_param("data empty bare content", msg, CMSG_BARE_CONTENT_PARAM,
     dataEmptyBareContent, sizeof(dataEmptyBareContent));
    check_param("data empty content", msg, CMSG_CONTENT_PARAM, dataEmptyContent,
     sizeof(dataEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("data bare content", msg, CMSG_BARE_CONTENT_PARAM,
     dataBareContent, sizeof(dataBareContent));
    check_param("data content", msg, CMSG_CONTENT_PARAM, dataContent,
     sizeof(dataContent));
    CryptMsgClose(msg);
    /* The inner OID is apparently ignored */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, oid,
     NULL);
    check_param("data bogus oid bare content", msg, CMSG_BARE_CONTENT_PARAM,
     dataEmptyBareContent, sizeof(dataEmptyBareContent));
    check_param("data bogus oid content", msg, CMSG_CONTENT_PARAM,
     dataEmptyContent, sizeof(dataEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("data bare content", msg, CMSG_BARE_CONTENT_PARAM,
     dataBareContent, sizeof(dataBareContent));
    check_param("data content", msg, CMSG_CONTENT_PARAM, dataContent,
     sizeof(dataContent));
    CryptMsgClose(msg);
    /* A streaming message is DER encoded if the length is not 0xffffffff, but
     * curiously, updates aren't validated to make sure they don't exceed the
     * stated length.  (The resulting output will of course fail to decode.)
     */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL,
     NULL, &streamInfo);
    CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    CryptMsgClose(msg);
    check_updates("bogus data message with definite length", &a1, &accum);
    free_updates(&accum);
    /* A valid definite-length encoding: */
    streamInfo.cbContent = sizeof(msgData);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL,
     NULL, &streamInfo);
    CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    CryptMsgClose(msg);
    check_updates("data message with definite length", &a2, &accum);
    free_updates(&accum);
    /* An indefinite-length encoding: */
    streamInfo.cbContent = 0xffffffff;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL,
     NULL, &streamInfo);
    CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    CryptMsgClose(msg);
    check_updates("data message with indefinite length", &a3, &accum);
    free_updates(&accum);
}

static void test_data_msg(void)
{
    test_data_msg_open();
    test_data_msg_update();
    test_data_msg_get_param();
    test_data_msg_encoding();
}

static void test_hash_msg_open(void)
{
    HCRYPTMSG msg;
    CMSG_HASHED_ENCODE_INFO hashInfo = { 0 };
    CMSG_STREAM_INFO streamInfo = { 0, nop_stream_output, NULL };

    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    hashInfo.cbSize = sizeof(hashInfo);
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == CRYPT_E_UNKNOWN_ALGO,
     "Expected CRYPT_E_UNKNOWN_ALGO, got %x\n", GetLastError());
    hashInfo.HashAlgorithm.pszObjId = oid_rsa_md5;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_HASHED, &hashInfo, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_HASHED, &hashInfo, NULL, &streamInfo);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);
}

static void test_hash_msg_update(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    CMSG_HASHED_ENCODE_INFO hashInfo = { sizeof(hashInfo), 0,
     { oid_rsa_md5, { 0, NULL } }, NULL };
    CMSG_STREAM_INFO streamInfo = { 0, nop_stream_output, NULL };

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_HASHED, &hashInfo, NULL, NULL);
    /* Detached hashed messages opened in non-streaming mode allow non-final
     * updates..
     */
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* including non-final updates with no data.. */
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* and final updates with no data. */
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* But no updates are allowed after the final update. */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* Non-detached messages, in contrast, don't allow non-final updates in
     * non-streaming mode.
     */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    /* Final updates (including empty ones) are allowed. */
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    /* And, of course, streaming mode allows non-final updates */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, &streamInfo);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    /* Setting pfnStreamOutput to NULL results in no error.  (In what appears
     * to be a bug, it isn't actually used - see encoding tests.)
     */
    streamInfo.pfnStreamOutput = NULL;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, &streamInfo);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);
}

static const BYTE emptyHashParam[] = {
0xd4,0x1d,0x8c,0xd9,0x8f,0x00,0xb2,0x04,0xe9,0x80,0x09,0x98,0xec,0xf8,0x42,
0x7e };

static void test_hash_msg_get_param(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    CMSG_HASHED_ENCODE_INFO hashInfo = { sizeof(hashInfo), 0,
     { oid_rsa_md5, { 0, NULL } }, NULL };
    DWORD size, value;
    CMSG_STREAM_INFO streamInfo = { 0, nop_stream_output, NULL };
    BYTE buf[16];

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, NULL);
    /* Content and bare content are always gettable for non-streamed messages */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_CONTENT_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_BARE_CONTENT_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    /* For an encoded hash message, the hash data aren't available */
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_HASH_DATA_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    /* The hash is also available. */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(size == sizeof(buf), "Unexpected size %d\n", size);
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 0, buf, &size);
    if (size == sizeof(buf))
        ok(!memcmp(buf, emptyHashParam, size), "Unexpected value\n");
    /* By getting the hash, further updates are not allowed */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(!ret && GetLastError() == NTE_BAD_HASH_STATE,
     "Expected NTE_BAD_HASH_STATE, got %x\n", GetLastError());
    /* Even after a final update, the hash data aren't available */
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_HASH_DATA_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    /* The version is also available, and should be zero for this message. */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_VERSION_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_VERSION_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == 0, "Expected version 0, got %d\n", value);
    /* As usual, the type isn't available. */
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, NULL, &size);
    ok(!ret, "Expected failure\n");
    CryptMsgClose(msg);

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, &streamInfo);
    /* Streamed messages don't allow you to get the content or bare content. */
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_CONTENT_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_BARE_CONTENT_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    /* The hash is still available. */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(size == sizeof(buf), "Unexpected size %d\n", size);
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 0, buf, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    if (size == sizeof(buf))
        ok(!memcmp(buf, emptyHashParam, size), "Unexpected value\n");
    /* After updating the hash, further updates aren't allowed on streamed
     * messages either.
     */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(!ret && GetLastError() == NTE_BAD_HASH_STATE,
     "Expected NTE_BAD_HASH_STATE, got %x\n", GetLastError());
    CryptMsgClose(msg);
}

static const BYTE hashEmptyBareContent[] = {
0x30,0x17,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0x04,0x00 };
static const BYTE hashEmptyContent[] = {
0x30,0x26,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x19,
0x30,0x17,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0x04,0x00 };
static const BYTE hashBareContent[] = {
0x30,0x38,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0x04,0x10,0x08,0xd6,0xc0,
0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,0x9d,0x2a,0x8f,0x26,0x2f };
static const BYTE hashContent[] = {
0x30,0x47,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x3a,
0x30,0x38,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0x04,0x10,0x08,0xd6,0xc0,
0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,0x9d,0x2a,0x8f,0x26,0x2f };

static const BYTE detachedHashNonFinalBareContent[] = {
0x30,0x20,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0x04,0x00 };
static const BYTE detachedHashNonFinalContent[] = {
0x30,0x2f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x22,
0x30,0x20,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0x04,0x00 };
static const BYTE detachedHashBareContent[] = {
0x30,0x30,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0x04,0x10,0x08,0xd6,0xc0,0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,
0x9d,0x2a,0x8f,0x26,0x2f };
static const BYTE detachedHashContent[] = {
0x30,0x3f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x32,
0x30,0x30,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0x04,0x10,0x08,0xd6,0xc0,0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,
0x9d,0x2a,0x8f,0x26,0x2f };

static void test_hash_msg_encoding(void)
{
    HCRYPTMSG msg;
    CMSG_HASHED_ENCODE_INFO hashInfo = { sizeof(hashInfo), 0 };
    BOOL ret;
    struct update_accum accum = { 0, NULL }, empty_accum = { 0, NULL };
    CMSG_STREAM_INFO streamInfo = { 0, accumulating_stream_output, &accum };

    hashInfo.HashAlgorithm.pszObjId = oid_rsa_md5;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, NULL);
    check_param("hash empty bare content", msg, CMSG_BARE_CONTENT_PARAM,
     hashEmptyBareContent, sizeof(hashEmptyBareContent));
    check_param("hash empty content", msg, CMSG_CONTENT_PARAM,
     hashEmptyContent, sizeof(hashEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("hash bare content", msg, CMSG_BARE_CONTENT_PARAM,
     hashBareContent, sizeof(hashBareContent));
    check_param("hash content", msg, CMSG_CONTENT_PARAM,
     hashContent, sizeof(hashContent));
    CryptMsgClose(msg);
    /* Same test, but with CMSG_BARE_CONTENT_FLAG set */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_BARE_CONTENT_FLAG,
     CMSG_HASHED, &hashInfo, NULL, NULL);
    check_param("hash empty bare content", msg, CMSG_BARE_CONTENT_PARAM,
     hashEmptyBareContent, sizeof(hashEmptyBareContent));
    check_param("hash empty content", msg, CMSG_CONTENT_PARAM,
     hashEmptyContent, sizeof(hashEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("hash bare content", msg, CMSG_BARE_CONTENT_PARAM,
     hashBareContent, sizeof(hashBareContent));
    check_param("hash content", msg, CMSG_CONTENT_PARAM,
     hashContent, sizeof(hashContent));
    CryptMsgClose(msg);
    /* Same test, but with CMSG_DETACHED_FLAG set */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_HASHED, &hashInfo, NULL, NULL);
    check_param("detached hash empty bare content", msg,
     CMSG_BARE_CONTENT_PARAM, hashEmptyBareContent,
     sizeof(hashEmptyBareContent));
    check_param("detached hash empty content", msg, CMSG_CONTENT_PARAM,
     hashEmptyContent, sizeof(hashEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("detached hash not final bare content", msg,
     CMSG_BARE_CONTENT_PARAM, detachedHashNonFinalBareContent,
     sizeof(detachedHashNonFinalBareContent));
    check_param("detached hash not final content", msg, CMSG_CONTENT_PARAM,
     detachedHashNonFinalContent, sizeof(detachedHashNonFinalContent));
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    check_param("detached hash bare content", msg, CMSG_BARE_CONTENT_PARAM,
     detachedHashBareContent, sizeof(detachedHashBareContent));
    check_param("detached hash content", msg, CMSG_CONTENT_PARAM,
     detachedHashContent, sizeof(detachedHashContent));
    check_param("detached hash bare content", msg, CMSG_BARE_CONTENT_PARAM,
     detachedHashBareContent, sizeof(detachedHashBareContent));
    check_param("detached hash content", msg, CMSG_CONTENT_PARAM,
     detachedHashContent, sizeof(detachedHashContent));
    CryptMsgClose(msg);
    /* In what appears to be a bug, streamed updates to hash messages don't
     * call the output function.
     */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, &streamInfo);
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    check_updates("empty hash message", &empty_accum, &accum);
    free_updates(&accum);

    streamInfo.cbContent = sizeof(msgData);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, &streamInfo);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    check_updates("hash message", &empty_accum, &accum);
    free_updates(&accum);

    streamInfo.cbContent = sizeof(msgData);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG,
     CMSG_HASHED, &hashInfo, NULL, &streamInfo);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    check_updates("detached hash message", &empty_accum, &accum);
    free_updates(&accum);
}

static void test_hash_msg(void)
{
    test_hash_msg_open();
    test_hash_msg_update();
    test_hash_msg_get_param();
    test_hash_msg_encoding();
}

static const WCHAR cspNameW[] = { 'W','i','n','e','C','r','y','p','t','T','e',
 'm','p',0 };
static BYTE serialNum[] = { 1 };
static BYTE encodedCommonName[] = { 0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,
 0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00 };

static void test_signed_msg_open(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    CMSG_SIGNED_ENCODE_INFO signInfo = { 0 };
    CMSG_SIGNER_ENCODE_INFO signer = { sizeof(signer), 0 };
    CERT_INFO certInfo = { 0 };

    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    signInfo.cbSize = sizeof(signInfo);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    signInfo.cSigners = 1;
    signInfo.rgSigners = &signer;
    /* With signer.pCertInfo unset, attempting to open this message this
     * crashes.
     */
    signer.pCertInfo = &certInfo;
    /* The cert info must contain a serial number and an issuer. */
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    certInfo.SerialNumber.cbData = sizeof(serialNum);
    certInfo.SerialNumber.pbData = serialNum;
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());
    certInfo.Issuer.cbData = sizeof(encodedCommonName);
    certInfo.Issuer.pbData = encodedCommonName;
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %x\n", GetLastError());

    /* The signer's hCryptProv must be set to something.  Whether it's usable
     * or not will be checked after the hash algorithm is checked (see next
     * test.)
     */
    signer.hCryptProv = 1;
    SetLastError(0xdeadbeef);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(!msg && GetLastError() == CRYPT_E_UNKNOWN_ALGO,
     "Expected CRYPT_E_UNKNOWN_ALGO, got %x\n", GetLastError());
    /* The signer's hash algorithm must also be set. */
    signer.HashAlgorithm.pszObjId = oid_rsa_md5;
    SetLastError(0xdeadbeef);
    /* Crashes in advapi32 in wine, don't do it */
    if (0) {
        msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED,
         &signInfo, NULL, NULL);
        ok(!msg && GetLastError() == ERROR_INVALID_PARAMETER,
         "Expected ERROR_INVALID_PARAMETER, got %x\n", GetLastError());
    }
    /* The signer's hCryptProv must also be valid. */
    ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
     PROV_RSA_FULL, CRYPT_NEWKEYSET);
    if (!ret && GetLastError() == NTE_EXISTS)
        ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
         PROV_RSA_FULL, 0);
    ok(ret, "CryptAcquireContextW failed: %x\n", GetLastError());
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    CryptReleaseContext(signer.hCryptProv, 0);
    pCryptAcquireContextW(&signer.hCryptProv, cspNameW, MS_DEF_PROV_W,
     PROV_RSA_FULL, CRYPT_DELETEKEYSET);
}

static const BYTE privKey[] = {
 0x07, 0x02, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x52, 0x53, 0x41, 0x32, 0x00,
 0x02, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x79, 0x10, 0x1c, 0xd0, 0x6b, 0x10,
 0x18, 0x30, 0x94, 0x61, 0xdc, 0x0e, 0xcb, 0x96, 0x4e, 0x21, 0x3f, 0x79, 0xcd,
 0xa9, 0x17, 0x62, 0xbc, 0xbb, 0x61, 0x4c, 0xe0, 0x75, 0x38, 0x6c, 0xf3, 0xde,
 0x60, 0x86, 0x03, 0x97, 0x65, 0xeb, 0x1e, 0x6b, 0xdb, 0x53, 0x85, 0xad, 0x68,
 0x21, 0xf1, 0x5d, 0xe7, 0x1f, 0xe6, 0x53, 0xb4, 0xbb, 0x59, 0x3e, 0x14, 0x27,
 0xb1, 0x83, 0xa7, 0x3a, 0x54, 0xe2, 0x8f, 0x65, 0x8e, 0x6a, 0x4a, 0xcf, 0x3b,
 0x1f, 0x65, 0xff, 0xfe, 0xf1, 0x31, 0x3a, 0x37, 0x7a, 0x8b, 0xcb, 0xc6, 0xd4,
 0x98, 0x50, 0x36, 0x67, 0xe4, 0xa1, 0xe8, 0x7e, 0x8a, 0xc5, 0x23, 0xf2, 0x77,
 0xf5, 0x37, 0x61, 0x49, 0x72, 0x59, 0xe8, 0x3d, 0xf7, 0x60, 0xb2, 0x77, 0xca,
 0x78, 0x54, 0x6d, 0x65, 0x9e, 0x03, 0x97, 0x1b, 0x61, 0xbd, 0x0c, 0xd8, 0x06,
 0x63, 0xe2, 0xc5, 0x48, 0xef, 0xb3, 0xe2, 0x6e, 0x98, 0x7d, 0xbd, 0x4e, 0x72,
 0x91, 0xdb, 0x31, 0x57, 0xe3, 0x65, 0x3a, 0x49, 0xca, 0xec, 0xd2, 0x02, 0x4e,
 0x22, 0x7e, 0x72, 0x8e, 0xf9, 0x79, 0x84, 0x82, 0xdf, 0x7b, 0x92, 0x2d, 0xaf,
 0xc9, 0xe4, 0x33, 0xef, 0x89, 0x5c, 0x66, 0x99, 0xd8, 0x80, 0x81, 0x47, 0x2b,
 0xb1, 0x66, 0x02, 0x84, 0x59, 0x7b, 0xc3, 0xbe, 0x98, 0x45, 0x4a, 0x3d, 0xdd,
 0xea, 0x2b, 0xdf, 0x4e, 0xb4, 0x24, 0x6b, 0xec, 0xe7, 0xd9, 0x0c, 0x45, 0xb8,
 0xbe, 0xca, 0x69, 0x37, 0x92, 0x4c, 0x38, 0x6b, 0x96, 0x6d, 0xcd, 0x86, 0x67,
 0x5c, 0xea, 0x54, 0x94, 0xa4, 0xca, 0xa4, 0x02, 0xa5, 0x21, 0x4d, 0xae, 0x40,
 0x8f, 0x9d, 0x51, 0x83, 0xf2, 0x3f, 0x33, 0xc1, 0x72, 0xb4, 0x1d, 0x94, 0x6e,
 0x7d, 0xe4, 0x27, 0x3f, 0xea, 0xff, 0xe5, 0x9b, 0xa7, 0x5e, 0x55, 0x8e, 0x0d,
 0x69, 0x1c, 0x7a, 0xff, 0x81, 0x9d, 0x53, 0x52, 0x97, 0x9a, 0x76, 0x79, 0xda,
 0x93, 0x32, 0x16, 0xec, 0x69, 0x51, 0x1a, 0x4e, 0xc3, 0xf1, 0x72, 0x80, 0x78,
 0x5e, 0x66, 0x4a, 0x8d, 0x85, 0x2f, 0x3f, 0xb2, 0xa7 };
static BYTE pubKey[] = {
0x30,0x48,0x02,0x41,0x00,0xe2,0x54,0x3a,0xa7,0x83,0xb1,0x27,0x14,0x3e,0x59,
0xbb,0xb4,0x53,0xe6,0x1f,0xe7,0x5d,0xf1,0x21,0x68,0xad,0x85,0x53,0xdb,0x6b,
0x1e,0xeb,0x65,0x97,0x03,0x86,0x60,0xde,0xf3,0x6c,0x38,0x75,0xe0,0x4c,0x61,
0xbb,0xbc,0x62,0x17,0xa9,0xcd,0x79,0x3f,0x21,0x4e,0x96,0xcb,0x0e,0xdc,0x61,
0x94,0x30,0x18,0x10,0x6b,0xd0,0x1c,0x10,0x79,0x02,0x03,0x01,0x00,0x01 };

static void test_signed_msg_update(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    CMSG_SIGNED_ENCODE_INFO signInfo = { sizeof(signInfo), 0 };
    CMSG_SIGNER_ENCODE_INFO signer = { sizeof(signer), 0 };
    CERT_INFO certInfo = { 0 };
    HCRYPTKEY key;

    certInfo.SerialNumber.cbData = sizeof(serialNum);
    certInfo.SerialNumber.pbData = serialNum;
    certInfo.Issuer.cbData = sizeof(encodedCommonName);
    certInfo.Issuer.pbData = encodedCommonName;
    signer.pCertInfo = &certInfo;
    signer.HashAlgorithm.pszObjId = oid_rsa_md5;
    signInfo.cSigners = 1;
    signInfo.rgSigners = &signer;
    ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
     PROV_RSA_FULL, CRYPT_NEWKEYSET);
    if (!ret && GetLastError() == NTE_EXISTS)
        ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
         PROV_RSA_FULL, 0);
    ok(ret, "CryptAcquireContextW failed: %x\n", GetLastError());
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING,
     CMSG_DETACHED_FLAG, CMSG_SIGNED, &signInfo, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    /* Detached CMSG_SIGNED allows non-final updates. */
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* Detached CMSG_SIGNED also allows non-final updates with no data. */
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* The final update requires a private key in the hCryptProv, in order to
     * generate the signature.
     */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(!ret && (GetLastError() == NTE_BAD_KEYSET ||
     GetLastError() == NTE_NO_KEY),
     "Expected NTE_BAD_KEYSET or NTE_NO_KEY, got %x\n", GetLastError());
    ret = CryptImportKey(signer.hCryptProv, (LPBYTE)privKey, sizeof(privKey),
     0, 0, &key);
    ok(ret, "CryptImportKey failed: %08x\n", GetLastError());
    /* The final update should be able to succeed now that a key exists, but
     * the previous (invalid) final update prevents it.
     */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING,
     CMSG_DETACHED_FLAG, CMSG_SIGNED, &signInfo, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    /* Detached CMSG_SIGNED allows non-final updates. */
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* Detached CMSG_SIGNED also allows non-final updates with no data. */
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* Now that the private key exists, the final update can succeed (even
     * with no data.)
     */
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* But no updates are allowed after the final update. */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    /* Non-detached messages don't allow non-final updates.. */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    /* but they do allow final ones. */
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    /* They also allow final updates with no data. */
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);

    CryptDestroyKey(key);
    CryptReleaseContext(signer.hCryptProv, 0);
    pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL, PROV_RSA_FULL,
     CRYPT_DELETEKEYSET);
}

static const BYTE signedEmptyBareContent[] = {
0x30,0x50,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0x31,0x37,0x30,0x35,0x02,
0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,
0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,
0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,
0x04,0x06,0x00,0x05,0x00,0x04,0x00 };
static const BYTE signedEmptyContent[] = {
0x30,0x5f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,0x52,
0x30,0x50,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0x31,0x37,0x30,0x35,0x02,
0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,
0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,
0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,
0x04,0x06,0x00,0x05,0x00,0x04,0x00 };
static const BYTE detachedSignedBareContent[] = {
0x30,0x81,0x99,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x01,0x07,0x01,0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,
0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,
0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,
0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,0xb6,
0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,0xee,
0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,0x92,
0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,0x44,
0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static const BYTE detachedSignedContent[] = {
0x30,0x81,0xaa,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0x9c,0x30,0x81,0x99,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,
0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,
0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,
0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,
0x05,0x00,0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,0xc0,
0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,
0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,0x23,
0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,0x51,
0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static const BYTE signedBareContent[] = {
0x30,0x81,0xa1,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0x31,0x77,
0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,
0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,
0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,
0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,
0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,
0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,
0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,
0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static const BYTE signedContent[] = {
0x30,0x81,0xb2,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xa4,0x30,0x81,0xa1,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,
0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0x81,0xa6,0x70,
0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,
0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,
0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,
0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,
0x0d };
static const BYTE signedHash[] = {
0x08,0xd6,0xc0,0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,0x9d,0x2a,0x8f,0x26,
0x2f };
static const BYTE signedEncodedSigner[] = {
0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,
0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,
0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,
0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,
0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,
0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,
0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,
0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static const BYTE signedWithAuthAttrsBareContent[] = {
0x30,0x82,0x01,0x00,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0x31,
0x81,0xd5,0x30,0x81,0xd2,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,
0x0d,0x02,0x05,0x05,0x00,0xa0,0x5b,0x30,0x18,0x06,0x09,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x01,0x09,0x03,0x31,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x01,0x07,0x01,0x30,0x1e,0x06,0x03,0x55,0x04,0x03,0x31,0x17,0x30,0x15,0x31,
0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,
0x4c,0x61,0x6e,0x67,0x00,0x30,0x1f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x01,0x09,0x04,0x31,0x12,0x04,0x10,0x08,0xd6,0xc0,0x5a,0x21,0x51,0x2a,0x79,
0xa1,0xdf,0xeb,0x9d,0x2a,0x8f,0x26,0x2f,0x30,0x04,0x06,0x00,0x05,0x00,0x04,
0x40,0xbf,0x65,0xde,0x7a,0x3e,0xa2,0x19,0x59,0xc3,0xc7,0x02,0x53,0xc9,0x72,
0xcd,0x74,0x96,0x70,0x0b,0x3b,0xcf,0x8b,0xd9,0x17,0x5c,0xc5,0xd1,0x83,0x41,
0x32,0x93,0xa6,0xf3,0x52,0x83,0x94,0xa9,0x6b,0x0a,0x92,0xcf,0xaf,0x12,0xfa,
0x40,0x53,0x12,0x84,0x03,0xab,0x10,0xa2,0x3d,0xe6,0x9f,0x5a,0xbf,0xc5,0xb8,
0xff,0xc6,0x33,0x63,0x34 };
static BYTE cert[] = {
0x30,0x7a,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,
0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,
0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,
0x00,0x30,0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,0xa3,0x16,0x30,0x14,0x30,
0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,
0xff,0x02,0x01,0x01 };
static BYTE v1CertWithPubKey[] = {
0x30,0x81,0x95,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,
0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,
0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x22,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x01,0x01,0x05,0x00,0x03,0x11,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xa3,0x16,0x30,0x14,0x30,0x12,0x06,
0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,0xff,0x02,
0x01,0x01 };
static const BYTE signedWithCertEmptyBareContent[] = {
0x30,0x81,0xce,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa0,0x7c,0x30,0x7a,
0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,
0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,
0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,
0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,
0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,
0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,0xa3,0x16,0x30,0x14,0x30,0x12,0x06,
0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,0xff,0x02,
0x01,0x01,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,
0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,
0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x00 };
static const BYTE signedWithCertBareContent[] = {
0x30,0x82,0x01,0x1f,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0xa0,
0x7c,0x30,0x7a,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,
0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,
0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,0xa3,0x16,0x30,0x14,
0x30,0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,
0x01,0xff,0x02,0x01,0x01,0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,
0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,
0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,
0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,0xb6,
0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,0xee,
0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,0x92,
0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,0x44,
0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static BYTE crl[] = { 0x30,0x2c,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,
0x30,0x30,0x30,0x30,0x5a };
static const BYTE signedWithCrlEmptyBareContent[] = {
0x30,0x81,0x80,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa1,0x2e,0x30,0x2c,
0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,
0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x18,0x0f,0x31,
0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x31,
0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,
0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,
0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,
0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x00 };
static const BYTE signedWithCrlBareContent[] = {
0x30,0x81,0xd1,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0xa1,0x2e,
0x30,0x2c,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,
0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x18,
0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,
0x5a,0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,
0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0x81,0xa6,
0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,
0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,
0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,
0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,
0xa8,0x0d };
static const BYTE signedWithCertAndCrlEmptyBareContent[] = {
0x30,0x81,0xfe,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa0,0x7c,0x30,0x7a,
0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,
0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,
0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,
0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,
0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,
0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,0xa3,0x16,0x30,0x14,0x30,0x12,0x06,
0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,0xff,0x02,
0x01,0x01,0xa1,0x2e,0x30,0x2c,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,
0x30,0x30,0x30,0x30,0x5a,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,
0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,
0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,
0x04,0x00 };
static const BYTE signedWithCertAndCrlBareContent[] = {
0x30,0x82,0x01,0x4f,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0xa0,
0x7c,0x30,0x7a,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,
0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,
0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,0xa3,0x16,0x30,0x14,
0x30,0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,
0x01,0xff,0x02,0x01,0x01,0xa1,0x2e,0x30,0x2c,0x30,0x02,0x06,0x00,0x30,0x15,
0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,
0x20,0x4c,0x61,0x6e,0x67,0x00,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,
0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x31,0x77,0x30,0x75,0x02,0x01,0x01,
0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,
0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,
0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,
0x00,0x05,0x00,0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,
0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,
0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,
0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,
0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static const BYTE signedWithCertWithPubKeyBareContent[] = {
0x30,0x81,0xeb,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa0,0x81,0x98,0x30,
0x81,0x95,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,
0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,
0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,
0x00,0x30,0x22,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,
0x01,0x05,0x00,0x03,0x11,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xa3,0x16,0x30,0x14,0x30,0x12,0x06,0x03,
0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,0xff,0x02,0x01,
0x01,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,
0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x00 };
static BYTE v1CertWithValidPubKey[] = {
0x30,0x81,0xcf,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,
0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,
0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x5c,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x01,0x01,0x05,0x00,0x03,0x4b,0x00,0x30,0x48,0x02,0x41,0x00,0xe2,0x54,0x3a,
0xa7,0x83,0xb1,0x27,0x14,0x3e,0x59,0xbb,0xb4,0x53,0xe6,0x1f,0xe7,0x5d,0xf1,
0x21,0x68,0xad,0x85,0x53,0xdb,0x6b,0x1e,0xeb,0x65,0x97,0x03,0x86,0x60,0xde,
0xf3,0x6c,0x38,0x75,0xe0,0x4c,0x61,0xbb,0xbc,0x62,0x17,0xa9,0xcd,0x79,0x3f,
0x21,0x4e,0x96,0xcb,0x0e,0xdc,0x61,0x94,0x30,0x18,0x10,0x6b,0xd0,0x1c,0x10,
0x79,0x02,0x03,0x01,0x00,0x01,0xa3,0x16,0x30,0x14,0x30,0x12,0x06,0x03,0x55,
0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,0xff,0x02,0x01,0x01 };
static const BYTE signedWithCertWithValidPubKeyEmptyContent[] = {
0x30,0x82,0x01,0x38,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x82,0x01,0x29,0x30,0x82,0x01,0x25,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,
0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,
0x00,0xa0,0x81,0xd2,0x30,0x81,0xcf,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,
0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,
0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,
0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,
0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,
0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,
0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x5c,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x4b,0x00,0x30,0x48,0x02,0x41,
0x00,0xe2,0x54,0x3a,0xa7,0x83,0xb1,0x27,0x14,0x3e,0x59,0xbb,0xb4,0x53,0xe6,
0x1f,0xe7,0x5d,0xf1,0x21,0x68,0xad,0x85,0x53,0xdb,0x6b,0x1e,0xeb,0x65,0x97,
0x03,0x86,0x60,0xde,0xf3,0x6c,0x38,0x75,0xe0,0x4c,0x61,0xbb,0xbc,0x62,0x17,
0xa9,0xcd,0x79,0x3f,0x21,0x4e,0x96,0xcb,0x0e,0xdc,0x61,0x94,0x30,0x18,0x10,
0x6b,0xd0,0x1c,0x10,0x79,0x02,0x03,0x01,0x00,0x01,0xa3,0x16,0x30,0x14,0x30,
0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,
0xff,0x02,0x01,0x01,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,
0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,
0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,
0x00 };
static const BYTE signedWithCertWithValidPubKeyContent[] = {
0x30,0x82,0x01,0x89,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x82,0x01,0x7a,0x30,0x82,0x01,0x76,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,
0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,
0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,
0x02,0x03,0x04,0xa0,0x81,0xd2,0x30,0x81,0xcf,0x02,0x01,0x01,0x30,0x02,0x06,
0x00,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,
0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,
0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,
0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,
0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,
0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x5c,0x30,0x0d,0x06,0x09,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x4b,0x00,0x30,0x48,
0x02,0x41,0x00,0xe2,0x54,0x3a,0xa7,0x83,0xb1,0x27,0x14,0x3e,0x59,0xbb,0xb4,
0x53,0xe6,0x1f,0xe7,0x5d,0xf1,0x21,0x68,0xad,0x85,0x53,0xdb,0x6b,0x1e,0xeb,
0x65,0x97,0x03,0x86,0x60,0xde,0xf3,0x6c,0x38,0x75,0xe0,0x4c,0x61,0xbb,0xbc,
0x62,0x17,0xa9,0xcd,0x79,0x3f,0x21,0x4e,0x96,0xcb,0x0e,0xdc,0x61,0x94,0x30,
0x18,0x10,0x6b,0xd0,0x1c,0x10,0x79,0x02,0x03,0x01,0x00,0x01,0xa3,0x16,0x30,
0x14,0x30,0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,
0x01,0x01,0xff,0x02,0x01,0x01,0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,
0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,
0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,
0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,
0x00,0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,
0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,
0xee,0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,
0x92,0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,
0x44,0xb8,0x0b,0x28,0xf4,0xa8,0x0d };

static void test_signed_msg_encoding(void)
{
    HCRYPTMSG msg;
    CMSG_SIGNED_ENCODE_INFO signInfo = { sizeof(signInfo), 0 };
    CMSG_SIGNER_ENCODE_INFO signer = { sizeof(signer), 0 };
    CERT_INFO certInfo = { 0 };
    CERT_BLOB encodedCert = { sizeof(cert), cert };
    CRL_BLOB encodedCrl = { sizeof(crl), crl };
    char oid_common_name[] = szOID_COMMON_NAME;
    CRYPT_ATTR_BLOB commonName = { sizeof(encodedCommonName),
     encodedCommonName };
    CRYPT_ATTRIBUTE attr = { oid_common_name, 1, &commonName };
    BOOL ret;
    HCRYPTKEY key;
    DWORD size;

    certInfo.SerialNumber.cbData = sizeof(serialNum);
    certInfo.SerialNumber.pbData = serialNum;
    certInfo.Issuer.cbData = sizeof(encodedCommonName);
    certInfo.Issuer.pbData = encodedCommonName;
    signer.pCertInfo = &certInfo;
    signer.HashAlgorithm.pszObjId = oid_rsa_md5;
    signInfo.cSigners = 1;
    signInfo.rgSigners = &signer;
    ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
     PROV_RSA_FULL, CRYPT_NEWKEYSET);
    if (!ret && GetLastError() == NTE_EXISTS)
        ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
         PROV_RSA_FULL, 0);
    ok(ret, "CryptAcquireContextW failed: %x\n", GetLastError());
    ret = CryptImportKey(signer.hCryptProv, (LPBYTE)privKey, sizeof(privKey),
     0, 0, &key);
    ok(ret, "CryptImportKey failed: %08x\n", GetLastError());

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING,
     CMSG_DETACHED_FLAG, CMSG_SIGNED, &signInfo, NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    check_param("detached signed empty bare content", msg,
     CMSG_BARE_CONTENT_PARAM, signedEmptyBareContent,
     sizeof(signedEmptyBareContent));
    check_param("detached signed empty content", msg, CMSG_CONTENT_PARAM,
     signedEmptyContent, sizeof(signedEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("detached signed hash", msg, CMSG_COMPUTED_HASH_PARAM,
     signedHash, sizeof(signedHash));
    check_param("detached signed bare content", msg, CMSG_BARE_CONTENT_PARAM,
     detachedSignedBareContent, sizeof(detachedSignedBareContent));
    check_param("detached signed content", msg, CMSG_CONTENT_PARAM,
     detachedSignedContent, sizeof(detachedSignedContent));
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 1, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_INDEX,
     "Expected CRYPT_E_INVALID_INDEX, got %x\n", GetLastError());
    check_param("detached signed encoded signer", msg, CMSG_ENCODED_SIGNER,
     signedEncodedSigner, sizeof(signedEncodedSigner));

    CryptMsgClose(msg);

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    check_param("signed empty bare content", msg, CMSG_BARE_CONTENT_PARAM,
     signedEmptyBareContent, sizeof(signedEmptyBareContent));
    check_param("signed empty content", msg, CMSG_CONTENT_PARAM,
     signedEmptyContent, sizeof(signedEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("signed bare content", msg, CMSG_BARE_CONTENT_PARAM,
     signedBareContent, sizeof(signedBareContent));
    check_param("signed content", msg, CMSG_CONTENT_PARAM,
     signedContent, sizeof(signedContent));

    CryptMsgClose(msg);

    signer.cAuthAttr = 1;
    signer.rgAuthAttr = &attr;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    check_param("signed with auth attrs bare content", msg,
     CMSG_BARE_CONTENT_PARAM, signedWithAuthAttrsBareContent,
     sizeof(signedWithAuthAttrsBareContent));

    CryptMsgClose(msg);

    signer.cAuthAttr = 0;
    signInfo.rgCertEncoded = &encodedCert;
    signInfo.cCertEncoded = 1;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    check_param("signed with cert empty bare content", msg,
     CMSG_BARE_CONTENT_PARAM, signedWithCertEmptyBareContent,
     sizeof(signedWithCertEmptyBareContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("signed with cert bare content", msg, CMSG_BARE_CONTENT_PARAM,
     signedWithCertBareContent, sizeof(signedWithCertBareContent));

    CryptMsgClose(msg);

    signInfo.cCertEncoded = 0;
    signInfo.rgCrlEncoded = &encodedCrl;
    signInfo.cCrlEncoded = 1;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    check_param("signed with crl empty bare content", msg,
     CMSG_BARE_CONTENT_PARAM, signedWithCrlEmptyBareContent,
     sizeof(signedWithCrlEmptyBareContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("signed with crl bare content", msg, CMSG_BARE_CONTENT_PARAM,
     signedWithCrlBareContent, sizeof(signedWithCrlBareContent));

    CryptMsgClose(msg);

    signInfo.cCertEncoded = 1;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    check_param("signed with cert and crl empty bare content", msg,
     CMSG_BARE_CONTENT_PARAM, signedWithCertAndCrlEmptyBareContent,
     sizeof(signedWithCertAndCrlEmptyBareContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    check_param("signed with cert and crl bare content", msg,
     CMSG_BARE_CONTENT_PARAM, signedWithCertAndCrlBareContent,
     sizeof(signedWithCertAndCrlBareContent));

    CryptMsgClose(msg);

    /* Test with a cert with a (bogus) public key */
    signInfo.cCrlEncoded = 0;
    encodedCert.cbData = sizeof(v1CertWithPubKey);
    encodedCert.pbData = v1CertWithPubKey;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    check_param("signedWithCertWithPubKeyBareContent", msg,
     CMSG_BARE_CONTENT_PARAM, signedWithCertWithPubKeyBareContent,
     sizeof(signedWithCertWithPubKeyBareContent));
    CryptMsgClose(msg);

    encodedCert.cbData = sizeof(v1CertWithValidPubKey);
    encodedCert.pbData = v1CertWithValidPubKey;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());
    check_param("signedWithCertWithValidPubKeyEmptyContent", msg,
     CMSG_CONTENT_PARAM, signedWithCertWithValidPubKeyEmptyContent,
     sizeof(signedWithCertWithValidPubKeyEmptyContent));
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    check_param("signedWithCertWithValidPubKeyContent", msg,
     CMSG_CONTENT_PARAM, signedWithCertWithValidPubKeyContent,
     sizeof(signedWithCertWithValidPubKeyContent));
    CryptMsgClose(msg);

    CryptDestroyKey(key);
    CryptReleaseContext(signer.hCryptProv, 0);
    pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL, PROV_RSA_FULL,
     CRYPT_DELETEKEYSET);
}

static void test_signed_msg_get_param(void)
{
    BOOL ret;
    HCRYPTMSG msg;
    DWORD size, value = 0;
    CMSG_SIGNED_ENCODE_INFO signInfo = { sizeof(signInfo), 0 };
    CMSG_SIGNER_ENCODE_INFO signer = { sizeof(signer), 0 };
    CERT_INFO certInfo = { 0 };

    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    /* Content and bare content are always gettable */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_CONTENT_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_BARE_CONTENT_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    /* For "signed" messages, so is the version. */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_VERSION_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_VERSION_PARAM, 0, (LPBYTE)&value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == CMSG_SIGNED_DATA_V1, "Expected version 1, got %d\n", value);
    /* But for this message, with no signers, the hash and signer aren't
     * available.
     */
    size = 0;
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_ENCODED_SIGNER, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_INDEX,
     "Expected CRYPT_E_INVALID_INDEX, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_INDEX,
     "Expected CRYPT_E_INVALID_INDEX, got %x\n", GetLastError());
    /* As usual, the type isn't available. */
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());

    CryptMsgClose(msg);

    certInfo.SerialNumber.cbData = sizeof(serialNum);
    certInfo.SerialNumber.pbData = serialNum;
    certInfo.Issuer.cbData = sizeof(encodedCommonName);
    certInfo.Issuer.pbData = encodedCommonName;
    signer.pCertInfo = &certInfo;
    signer.HashAlgorithm.pszObjId = oid_rsa_md5;
    signInfo.cSigners = 1;
    signInfo.rgSigners = &signer;
    ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
     PROV_RSA_FULL, CRYPT_NEWKEYSET);
    if (!ret && GetLastError() == NTE_EXISTS)
        ret = pCryptAcquireContextW(&signer.hCryptProv, cspNameW, NULL,
         PROV_RSA_FULL, 0);
    ok(ret, "CryptAcquireContextW failed: %x\n", GetLastError());
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    ok(msg != NULL, "CryptMsgOpenToEncode failed: %x\n", GetLastError());

    /* This message, with one signer, has the hash and signer for index 0
     * available, but not for other indexes.
     */
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_ENCODED_SIGNER, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %x\n", GetLastError());
    size = 0;
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_ENCODED_SIGNER, 1, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_INDEX,
     "Expected CRYPT_E_INVALID_INDEX, got %x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 1, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_INDEX,
     "Expected CRYPT_E_INVALID_INDEX, got %x\n", GetLastError());
    /* As usual, the type isn't available. */
    ret = CryptMsgGetParam(msg, CMSG_TYPE_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());

    CryptMsgClose(msg);

    CryptReleaseContext(signer.hCryptProv, 0);
    pCryptAcquireContextW(&signer.hCryptProv, cspNameW, MS_DEF_PROV_W,
     PROV_RSA_FULL, CRYPT_DELETEKEYSET);
}

static void test_signed_msg(void)
{
    test_signed_msg_open();
    test_signed_msg_update();
    test_signed_msg_encoding();
    test_signed_msg_get_param();
}

static CRYPT_DATA_BLOB b4 = { 0, NULL };
static const struct update_accum a4 = { 1, &b4 };

static const BYTE bogusOIDContent[] = {
0x30,0x0f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x07,0xa0,0x02,
0x04,0x00 };
static const BYTE bogusHashContent[] = {
0x30,0x47,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x3a,
0x30,0x38,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,0x04,0x10,0x00,0xd6,0xc0,
0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,0x9d,0x2a,0x8f,0x26,0x2f };

static void test_decode_msg_update(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    CMSG_STREAM_INFO streamInfo = { 0 };
    DWORD i;
    struct update_accum accum = { 0, NULL };

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    /* Update with a full message in a final update */
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* Can't update after a final update */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    /* Can't send a non-final update without streaming */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent),
     FALSE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "Expected CRYPT_E_MSG_ERROR, got %x\n", GetLastError());
    /* A subsequent final update succeeds */
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, &streamInfo);
    /* Updating a message that has a NULL stream callback fails */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent),
     FALSE);
    todo_wine
    ok(!ret && GetLastError() == STATUS_ACCESS_VIOLATION,
     "Expected STATUS_ACCESS_VIOLATION, got %x\n", GetLastError());
    /* Changing the callback pointer after the fact yields the same error (so
     * the message must copy the stream info, not just store a pointer to it)
     */
    streamInfo.pfnStreamOutput = nop_stream_output;
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent),
     FALSE);
    todo_wine
    ok(!ret && GetLastError() == STATUS_ACCESS_VIOLATION,
     "Expected STATUS_ACCESS_VIOLATION, got %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Empty non-final updates are allowed when streaming.. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, &streamInfo);
    ret = CryptMsgUpdate(msg, NULL, 0, FALSE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    /* but final updates aren't when not enough data has been received. */
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    todo_wine
    ok(!ret && GetLastError() == CRYPT_E_STREAM_INSUFFICIENT_DATA,
     "Expected CRYPT_E_STREAM_INSUFFICIENT_DATA, got %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Updating the message byte by byte is legal */
    streamInfo.pfnStreamOutput = accumulating_stream_output;
    streamInfo.pvArg = &accum;
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, &streamInfo);
    for (i = 0, ret = TRUE; ret && i < sizeof(dataEmptyContent); i++)
        ret = CryptMsgUpdate(msg, &dataEmptyContent[i], 1, FALSE);
    ok(ret, "CryptMsgUpdate failed on byte %d: %x\n", i, GetLastError());
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    ok(ret, "CryptMsgUpdate failed on byte %d: %x\n", i, GetLastError());
    CryptMsgClose(msg);
    todo_wine
    check_updates("byte-by-byte empty content", &a4, &accum);
    free_updates(&accum);

    /* Decoding bogus content fails in non-streaming mode.. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* and as the final update in streaming mode.. */
    streamInfo.pfnStreamOutput = nop_stream_output;
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, &streamInfo);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), TRUE);
    todo_wine
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* and even as a non-final update in streaming mode. */
    streamInfo.pfnStreamOutput = nop_stream_output;
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, &streamInfo);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, msgData, sizeof(msgData), FALSE);
    todo_wine
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);

    /* An empty message can be opened with undetermined type.. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent),
     TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);
    /* but decoding it as an explicitly typed message fails. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, 0, NULL,
     NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, dataEmptyContent, sizeof(dataEmptyContent),
     TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* On the other hand, decoding the bare content of an empty message fails
     * with unspecified type..
     */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, dataEmptyBareContent,
     sizeof(dataEmptyBareContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* but succeeds with explicit type. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, 0, NULL,
     NULL);
    ret = CryptMsgUpdate(msg, dataEmptyBareContent,
     sizeof(dataEmptyBareContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Decoding valid content with an unsupported OID fails */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, bogusOIDContent, sizeof(bogusOIDContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Similarly, opening an empty hash with unspecified type succeeds.. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, hashEmptyContent, sizeof(hashEmptyContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);
    /* while with specified type it fails. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, hashEmptyContent, sizeof(hashEmptyContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* On the other hand, decoding the bare content of an empty hash message
     * fails with unspecified type..
     */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, hashEmptyBareContent,
     sizeof(hashEmptyBareContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* but succeeds with explicit type. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    ret = CryptMsgUpdate(msg, hashEmptyBareContent,
     sizeof(hashEmptyBareContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    /* And again, opening a (non-empty) hash message with unspecified type
     * succeeds..
     */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, hashContent, sizeof(hashContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);
    /* while with specified type it fails.. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, hashContent, sizeof(hashContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* and decoding the bare content of a non-empty hash message fails with
     * unspecified type..
     */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, hashBareContent, sizeof(hashBareContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %x\n", GetLastError());
    CryptMsgClose(msg);
    /* but succeeds with explicit type. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    ret = CryptMsgUpdate(msg, hashBareContent, sizeof(hashBareContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %x\n", GetLastError());
    CryptMsgClose(msg);

    /* Opening a (non-empty) hash message with unspecified type and a bogus
     * hash value succeeds..
     */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, bogusHashContent, sizeof(bogusHashContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ret = CryptMsgUpdate(msg, signedContent, sizeof(signedContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgUpdate(msg, signedWithCertAndCrlBareContent,
     sizeof(signedWithCertAndCrlBareContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, 0, NULL,
     NULL);
    ret = CryptMsgUpdate(msg, signedWithCertAndCrlBareContent,
     sizeof(signedWithCertAndCrlBareContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, CMSG_DETACHED_FLAG, 0, 0,
     NULL, NULL);
    /* The first update succeeds.. */
    ret = CryptMsgUpdate(msg, detachedSignedContent,
     sizeof(detachedSignedContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    /* as does a second (probably to update the detached portion).. */
    ret = CryptMsgUpdate(msg, detachedSignedContent,
     sizeof(detachedSignedContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    /* while a third fails. */
    ret = CryptMsgUpdate(msg, detachedSignedContent,
     sizeof(detachedSignedContent), TRUE);
    ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
     "expected CRYPT_E_MSG_ERROR, got %08x\n", GetLastError());
    CryptMsgClose(msg);
}

static const BYTE hashParam[] = { 0x08,0xd6,0xc0,0x5a,0x21,0x51,0x2a,0x79,0xa1,
 0xdf,0xeb,0x9d,0x2a,0x8f,0x26,0x2f };

static void compare_signer_info(const CMSG_SIGNER_INFO *got,
 const CMSG_SIGNER_INFO *expected)
{
    ok(got->dwVersion == expected->dwVersion, "Expected version %d, got %d\n",
     expected->dwVersion, got->dwVersion);
    ok(got->Issuer.cbData == expected->Issuer.cbData,
     "Expected issuer size %d, got %d\n", expected->Issuer.cbData,
     got->Issuer.cbData);
    ok(!memcmp(got->Issuer.pbData, got->Issuer.pbData, got->Issuer.cbData),
     "Unexpected issuer\n");
    ok(got->SerialNumber.cbData == expected->SerialNumber.cbData,
     "Expected serial number size %d, got %d\n", expected->SerialNumber.cbData,
     got->SerialNumber.cbData);
    ok(!memcmp(got->SerialNumber.pbData, got->SerialNumber.pbData,
     got->SerialNumber.cbData), "Unexpected serial number\n");
    /* FIXME: check more things */
}

static const BYTE signedWithCertAndCrlComputedHash[] = {
0x08,0xd6,0xc0,0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,0x9d,0x2a,0x8f,0x26,
0x2f };

static void test_decode_msg_get_param(void)
{
    HCRYPTMSG msg;
    BOOL ret;
    DWORD size = 0, value;
    LPBYTE buf;

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    SetLastError(0xdeadbeef);
    ret = CryptMsgGetParam(msg, CMSG_CONTENT_PARAM, 0, NULL, &size);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %x\n", GetLastError());
    ret = CryptMsgUpdate(msg, dataContent, sizeof(dataContent), TRUE);
    check_param("data content", msg, CMSG_CONTENT_PARAM, msgData,
     sizeof(msgData));
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ret = CryptMsgUpdate(msg, hashEmptyContent, sizeof(hashEmptyContent), TRUE);
    check_param("empty hash content", msg, CMSG_CONTENT_PARAM, NULL, 0);
    check_param("empty hash hash data", msg, CMSG_HASH_DATA_PARAM, NULL, 0);
    check_param("empty hash computed hash", msg, CMSG_COMPUTED_HASH_PARAM,
     emptyHashParam, sizeof(emptyHashParam));
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ret = CryptMsgUpdate(msg, hashContent, sizeof(hashContent), TRUE);
    check_param("hash content", msg, CMSG_CONTENT_PARAM, msgData,
     sizeof(msgData));
    check_param("hash hash data", msg, CMSG_HASH_DATA_PARAM, hashParam,
     sizeof(hashParam));
    check_param("hash computed hash", msg, CMSG_COMPUTED_HASH_PARAM,
     hashParam, sizeof(hashParam));
    /* Curiously, getting the hash of index 1 succeeds, even though there's
     * only one hash.
     */
    ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 1, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    buf = CryptMemAlloc(size);
    if (buf)
    {
        ret = CryptMsgGetParam(msg, CMSG_COMPUTED_HASH_PARAM, 1, buf, &size);
        ok(size == sizeof(hashParam), "Unexpected size %d\n", size);
        ok(!memcmp(buf, hashParam, size), "Unexpected value\n");
        CryptMemFree(buf);
    }
    check_param("hash inner OID", msg, CMSG_INNER_CONTENT_TYPE_PARAM,
     (const BYTE *)szOID_RSA_data, strlen(szOID_RSA_data) + 1);
    value = CMSG_HASHED_DATA_V0;
    check_param("hash version", msg, CMSG_VERSION_PARAM, (const BYTE *)&value,
     sizeof(value));
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    ret = CryptMsgUpdate(msg, signedContent, sizeof(signedContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    check_param("signed content", msg, CMSG_CONTENT_PARAM, msgData,
     sizeof(msgData));
    check_param("inner content", msg, CMSG_INNER_CONTENT_TYPE_PARAM,
     (const BYTE *)szOID_RSA_data, strlen(szOID_RSA_data) + 1);
    size = sizeof(value);
    value = 2112;
    ret = CryptMsgGetParam(msg, CMSG_SIGNER_COUNT_PARAM, 0, &value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == 1, "Expected 1 signer, got %d\n", value);
    size = 0;
    ret = CryptMsgGetParam(msg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    if (ret)
        buf = CryptMemAlloc(size);
    else
        buf = NULL;
    if (buf)
    {
        CMSG_SIGNER_INFO signer = { 0 };

        signer.dwVersion = 1;
        signer.Issuer.cbData = sizeof(encodedCommonName);
        signer.Issuer.pbData = encodedCommonName;
        signer.SerialNumber.cbData = sizeof(serialNum);
        signer.SerialNumber.pbData = serialNum;
        signer.HashAlgorithm.pszObjId = oid_rsa_md5;
        CryptMsgGetParam(msg, CMSG_SIGNER_INFO_PARAM, 0, buf, &size);
        compare_signer_info((CMSG_SIGNER_INFO *)buf, &signer);
        CryptMemFree(buf);
    }
    /* index is ignored when getting signer count */
    size = sizeof(value);
    ret = CryptMsgGetParam(msg, CMSG_SIGNER_COUNT_PARAM, 1, &value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == 1, "Expected 1 signer, got %d\n", value);
    ret = CryptMsgGetParam(msg, CMSG_CERT_COUNT_PARAM, 0, &value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == 0, "Expected 0 certs, got %d\n", value);
    ret = CryptMsgGetParam(msg, CMSG_CRL_COUNT_PARAM, 0, &value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == 0, "Expected 0 CRLs, got %d\n", value);
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, 0, NULL,
     NULL);
    ret = CryptMsgUpdate(msg, signedWithCertAndCrlBareContent,
     sizeof(signedWithCertAndCrlBareContent), TRUE);
    ok(ret, "CryptMsgUpdate failed: %08x\n", GetLastError());
    ret = CryptMsgGetParam(msg, CMSG_CERT_COUNT_PARAM, 0, &value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == 1, "Expected 1 cert, got %d\n", value);
    check_param("cert", msg, CMSG_CERT_PARAM, cert, sizeof(cert));
    ret = CryptMsgGetParam(msg, CMSG_CRL_COUNT_PARAM, 0, &value, &size);
    ok(ret, "CryptMsgGetParam failed: %08x\n", GetLastError());
    ok(value == 1, "Expected 1 CRL, got %d\n", value);
    check_param("crl", msg, CMSG_CRL_PARAM, crl, sizeof(crl));
    check_param("signed with cert and CRL computed hash", msg,
     CMSG_COMPUTED_HASH_PARAM, signedWithCertAndCrlComputedHash,
     sizeof(signedWithCertAndCrlComputedHash));
    CryptMsgClose(msg);
}

static void test_decode_msg(void)
{
    test_decode_msg_update();
    test_decode_msg_get_param();
}

static BYTE aKey[] = { 0,1,2,3,4,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf };
/* aKey encoded as a X509_PUBLIC_KEY_INFO */
static BYTE encodedPubKey[] = {
0x30,0x1f,0x30,0x0a,0x06,0x06,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x05,0x00,0x03,
0x11,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
0x0d,0x0e,0x0f };
/* a weird modulus encoded as RSA_CSP_PUBLICKEYBLOB */
static BYTE mod_encoded[] = {
 0x30,0x10,0x02,0x09,0x00,0x80,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x02,0x03,
 0x01,0x00,0x01 };

static void test_msg_control(void)
{
    static char oid_rsa_rsa[] = szOID_RSA_RSA;
    BOOL ret;
    HCRYPTMSG msg;
    DWORD i;
    CERT_INFO certInfo = { 0 };
    CMSG_HASHED_ENCODE_INFO hashInfo = { 0 };
    CMSG_SIGNED_ENCODE_INFO signInfo = { sizeof(signInfo), 0 };
    CMSG_CTRL_DECRYPT_PARA decryptPara = { sizeof(decryptPara), 0 };

    /* Crashes
    ret = CryptMsgControl(NULL, 0, 0, NULL);
    */

    /* Data encode messages don't allow any sort of control.. */
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_DATA, NULL, NULL,
     NULL);
    /* either with no prior update.. */
    for (i = 1; i <= CMSG_CTRL_ADD_CMS_SIGNER_INFO; i++)
    {
        SetLastError(0xdeadbeef);
        ret = CryptMsgControl(msg, 0, i, NULL);
        ok(!ret && GetLastError() == E_INVALIDARG,
         "Expected E_INVALIDARG, got %08x\n", GetLastError());
    }
    /* or after an update. */
    for (i = 1; i <= CMSG_CTRL_ADD_CMS_SIGNER_INFO; i++)
    {
        SetLastError(0xdeadbeef);
        ret = CryptMsgControl(msg, 0, i, NULL);
        ok(!ret && GetLastError() == E_INVALIDARG,
         "Expected E_INVALIDARG, got %08x\n", GetLastError());
    }
    CryptMsgClose(msg);

    /* Hash encode messages don't allow any sort of control.. */
    hashInfo.cbSize = sizeof(hashInfo);
    hashInfo.HashAlgorithm.pszObjId = oid_rsa_md5;
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, &hashInfo,
     NULL, NULL);
    /* either with no prior update.. */
    for (i = 1; i <= CMSG_CTRL_ADD_CMS_SIGNER_INFO; i++)
    {
        SetLastError(0xdeadbeef);
        ret = CryptMsgControl(msg, 0, i, NULL);
        ok(!ret && GetLastError() == E_INVALIDARG,
         "Expected E_INVALIDARG, got %08x\n", GetLastError());
    }
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    /* or after an update. */
    for (i = 1; i <= CMSG_CTRL_ADD_CMS_SIGNER_INFO; i++)
    {
        SetLastError(0xdeadbeef);
        ret = CryptMsgControl(msg, 0, i, NULL);
        ok(!ret && GetLastError() == E_INVALIDARG,
         "Expected E_INVALIDARG, got %08x\n", GetLastError());
    }
    CryptMsgClose(msg);

    /* Signed encode messages likewise don't allow any sort of control.. */
    signInfo.cbSize = sizeof(signInfo);
    msg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, &signInfo,
     NULL, NULL);
    /* either before an update.. */
    for (i = 1; i <= CMSG_CTRL_ADD_CMS_SIGNER_INFO; i++)
    {
        SetLastError(0xdeadbeef);
        ret = CryptMsgControl(msg, 0, i, NULL);
        ok(!ret && GetLastError() == E_INVALIDARG,
         "Expected E_INVALIDARG, got %08x\n", GetLastError());
    }
    ret = CryptMsgUpdate(msg, NULL, 0, TRUE);
    /* or after an update. */
    for (i = 1; i <= CMSG_CTRL_ADD_CMS_SIGNER_INFO; i++)
    {
        SetLastError(0xdeadbeef);
        ret = CryptMsgControl(msg, 0, i, NULL);
        ok(!ret && GetLastError() == E_INVALIDARG,
         "Expected E_INVALIDARG, got %08x\n", GetLastError());
    }
    CryptMsgClose(msg);

    /* Decode messages behave a bit differently. */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    /* Bad control type */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_CONTROL_TYPE,
     "Expected CRYPT_E_CONTROL_TYPE, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 1, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_CONTROL_TYPE,
     "Expected CRYPT_E_CONTROL_TYPE, got %08x\n", GetLastError());
    /* Can't verify the hash of an indeterminate-type message */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_HASH, NULL);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    /* Crashes
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_DECRYPT, NULL);
     */
    /* Can't decrypt an indeterminate-type message */
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_DECRYPT, &decryptPara);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    /* Can't verify the hash of an empty message */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_HASH, NULL);
    todo_wine
    ok(!ret && GetLastError() == STATUS_ACCESS_VIOLATION,
     "Expected STATUS_ACCESS_VIOLATION, got %08x\n", GetLastError());
    /* Crashes
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, NULL);
     */
    /* Can't verify the signature of a hash message */
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    CryptMsgUpdate(msg, hashEmptyBareContent, sizeof(hashEmptyBareContent),
     TRUE);
    /* Oddly enough, this fails */
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_HASH, NULL);
    todo_wine
    ok(!ret, "Expected failure\n");
    CryptMsgClose(msg);
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_HASHED, 0, NULL,
     NULL);
    CryptMsgUpdate(msg, hashBareContent, sizeof(hashBareContent), TRUE);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_HASH, NULL);
    ok(ret, "CryptMsgControl failed: %08x\n", GetLastError());
    /* Can't decrypt an indeterminate-type message */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_DECRYPT, &decryptPara);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    CryptMsgClose(msg);

    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, 0, NULL,
     NULL);
    /* Can't verify the hash of a signed message */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_HASH, NULL);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    /* Can't decrypt a signed message */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_DECRYPT, &decryptPara);
    ok(!ret && GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    /* Crash
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, NULL);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
     */
    CryptMsgUpdate(msg, signedWithCertBareContent,
     sizeof(signedWithCertBareContent), TRUE);
    /* With an empty cert info, the signer can't be found in the message (and
     * the signature can't be verified.
     */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(!ret && GetLastError() == CRYPT_E_SIGNER_NOT_FOUND,
     "Expected CRYPT_E_SIGNER_NOT_FOUND, got %08x\n", GetLastError());
    /* The cert info is expected to have an issuer, serial number, and public
     * key info set.
     */
    certInfo.SerialNumber.cbData = sizeof(serialNum);
    certInfo.SerialNumber.pbData = serialNum;
    certInfo.Issuer.cbData = sizeof(encodedCommonName);
    certInfo.Issuer.pbData = encodedCommonName;
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_EOD,
     "Expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    CryptMsgClose(msg);
    /* This cert has a public key, but it's not in a usable form */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, CMSG_SIGNED, 0, NULL,
     NULL);
    CryptMsgUpdate(msg, signedWithCertWithPubKeyBareContent,
     sizeof(signedWithCertWithPubKeyBareContent), TRUE);
    /* Again, cert info needs to have a public key set */
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_EOD,
     "Expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    /* The public key is supposed to be in encoded form.. */
    certInfo.SubjectPublicKeyInfo.Algorithm.pszObjId = oid_rsa_rsa;
    certInfo.SubjectPublicKeyInfo.PublicKey.cbData = sizeof(aKey);
    certInfo.SubjectPublicKeyInfo.PublicKey.pbData = aKey;
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    /* but not as a X509_PUBLIC_KEY_INFO.. */
    certInfo.SubjectPublicKeyInfo.Algorithm.pszObjId = NULL;
    certInfo.SubjectPublicKeyInfo.PublicKey.cbData = sizeof(encodedPubKey);
    certInfo.SubjectPublicKeyInfo.PublicKey.pbData = encodedPubKey;
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    /* This decodes successfully, but it doesn't match any key in the message */
    certInfo.SubjectPublicKeyInfo.PublicKey.cbData = sizeof(mod_encoded);
    certInfo.SubjectPublicKeyInfo.PublicKey.pbData = mod_encoded;
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    /* In Wine's rsaenh, this fails to decode because the key length is too
     * small.  Not sure if that's a bug in rsaenh, so leaving todo_wine for
     * now.
     */
    todo_wine
    ok(!ret && GetLastError() == NTE_BAD_SIGNATURE,
     "Expected NTE_BAD_SIGNATURE, got %08x\n", GetLastError());
    CryptMsgClose(msg);
    /* A message with no data doesn't have a valid signature */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    CryptMsgUpdate(msg, signedWithCertWithValidPubKeyEmptyContent,
     sizeof(signedWithCertWithValidPubKeyEmptyContent), TRUE);
    certInfo.SubjectPublicKeyInfo.Algorithm.pszObjId = oid_rsa_rsa;
    certInfo.SubjectPublicKeyInfo.PublicKey.cbData = sizeof(pubKey);
    certInfo.SubjectPublicKeyInfo.PublicKey.pbData = pubKey;
    SetLastError(0xdeadbeef);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(!ret && GetLastError() == NTE_BAD_SIGNATURE,
     "Expected NTE_BAD_SIGNATURE, got %08x\n", GetLastError());
    CryptMsgClose(msg);
    /* Finally, this succeeds */
    msg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0, NULL, NULL);
    CryptMsgUpdate(msg, signedWithCertWithValidPubKeyContent,
     sizeof(signedWithCertWithValidPubKeyContent), TRUE);
    ret = CryptMsgControl(msg, 0, CMSG_CTRL_VERIFY_SIGNATURE, &certInfo);
    ok(ret, "CryptMsgControl failed: %08x\n", GetLastError());
    CryptMsgClose(msg);
}

static void test_msg_get_signer_count(void)
{
    LONG count;

    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(0, NULL, 0);
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == E_INVALIDARG, "Expected E_INVALIDARG, got %08x\n",
     GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING, NULL, 0);
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_ASN1_EOD,
     "Expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     dataEmptyBareContent, sizeof(dataEmptyBareContent));
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     dataEmptyContent, sizeof(dataEmptyContent));
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     signedEmptyBareContent, sizeof(signedEmptyBareContent));
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     signedEmptyContent, sizeof(signedEmptyContent));
    ok(count == 1, "Expected 1, got %d\n", count);
}

static const BYTE signedWithCertEmptyContent[] = {
0x30,0x81,0xdf,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xd1,0x30,0x81,0xce,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa0,0x7c,
0x30,0x7a,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,
0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,
0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,
0x00,0x30,0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,0xa3,0x16,0x30,0x14,0x30,
0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,
0xff,0x02,0x01,0x01,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,
0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,
0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,
0x00 };
static const BYTE signedWithCertContent[] = {
0x30,0x82,0x01,0x32,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x82,0x01,0x23,0x30,0x82,0x01,0x1f,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,
0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,
0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,
0x02,0x03,0x04,0xa0,0x7c,0x30,0x7a,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,
0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,
0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,
0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,
0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,
0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,
0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,
0xa3,0x16,0x30,0x14,0x30,0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,
0x08,0x30,0x06,0x01,0x01,0xff,0x02,0x01,0x01,0x31,0x77,0x30,0x75,0x02,0x01,
0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,
0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,
0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,
0x06,0x00,0x05,0x00,0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,
0x9b,0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,
0x2c,0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,
0x1d,0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,
0x39,0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static const BYTE signedWithCertWithPubKeyContent[] = {
0x30,0x81,0xfc,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xee,0x30,0x81,0xeb,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa0,0x81,
0x98,0x30,0x81,0x95,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,
0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,
0x61,0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,
0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,
0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x30,0x22,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x01,0x01,0x01,0x05,0x00,0x03,0x11,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xa3,0x16,0x30,0x14,0x30,0x12,
0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,0xff,
0x02,0x01,0x01,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,
0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,
0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x00 };

static void test_verify_message_signature(void)
{
    BOOL ret;
    CRYPT_VERIFY_MESSAGE_PARA para = { 0 };
    PCCERT_CONTEXT cert;
    DWORD cbDecoded;

    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(NULL, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    para.cbSize = sizeof(para);
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    para.cbSize = 0;
    para.dwMsgAndCertEncodingType = PKCS_7_ASN_ENCODING;
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    para.cbSize = sizeof(para);
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_EOD,
     "Expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    /* Check whether cert is set on error */
    cert = (PCCERT_CONTEXT)0xdeadbeef;
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, &cert);
    ok(cert == NULL, "Expected NULL cert\n");
    /* Check whether cbDecoded is set on error */
    cbDecoded = 0xdeadbeef;
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, &cbDecoded,
     NULL);
    ok(!cbDecoded, "Expected 0\n");
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, dataEmptyBareContent,
     sizeof(dataEmptyBareContent), NULL, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, dataEmptyContent,
     sizeof(dataEmptyContent), NULL, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_UNEXPECTED_MSG_TYPE,
     "Expected CRYPT_E_UNEXPECTED_MSG_TYPE, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, signedEmptyBareContent,
     sizeof(signedEmptyBareContent), NULL, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_ASN1_BADTAG,
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, signedEmptyContent,
     sizeof(signedEmptyContent), NULL, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_NOT_FOUND,
     "Expected CRYPT_E_NOT_FOUND, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, signedContent,
     sizeof(signedContent), NULL, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_NOT_FOUND,
     "Expected CRYPT_E_NOT_FOUND, got %08x\n", GetLastError());
    /* FIXME: Windows fails with CRYPT_E_NOT_FOUND for these messages, but
     * their signer certs have invalid public keys that fail to decode.  In
     * Wine therefore the failure is an ASN error.  Need some messages with
     * valid public keys and invalid signatures to check against.
     */
    ret = CryptVerifyMessageSignature(&para, 0, signedWithCertEmptyContent,
     sizeof(signedWithCertEmptyContent), NULL, 0, NULL);
    ok(!ret, "Expected failure\n");
    ret = CryptVerifyMessageSignature(&para, 0, signedWithCertContent,
     sizeof(signedWithCertContent), NULL, 0, NULL);
    ok(!ret, "Expected failure\n");
    ret = CryptVerifyMessageSignature(&para, 0, signedWithCertWithPubKeyContent,
     sizeof(signedWithCertWithPubKeyContent), NULL, 0, NULL);
    ok(!ret, "Expected failure\n");
}

START_TEST(msg)
{
     init_function_pointers();

    /* Basic parameter checking tests */
    test_msg_open_to_encode();
    test_msg_open_to_decode();
    test_msg_get_param();
    test_msg_close();
    test_msg_control();

    /* Message-type specific tests */
    test_data_msg();
    test_hash_msg();
    test_signed_msg();
    test_decode_msg();

    /* simplified message functions */
    test_msg_get_signer_count();
    test_verify_message_signature();
}
