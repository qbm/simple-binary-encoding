/*
 * Copyright 2013 Real Logic Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <iostream>
#include <sys/types.h>

#include "otf_api/Ir.h"
#include "uk_co_real_logic_sbe_ir_generated/FrameCodec.hpp"
#include "uk_co_real_logic_sbe_ir_generated/TokenCodec.hpp"

using namespace sbe::on_the_fly;
using namespace uk_co_real_logic_sbe_ir_generated;
using ::std::cout;
using ::std::endl;

struct Ir::Impl
{
    TokenCodec tokenCodec;
    char name[256];
    char constVal[256];
    char minVal[256];
    char maxVal[256];
    char nullVal[256];
    char characterEncoding[256];
    char epoch[256];
    char timeUnit[256];
    char semanticType[256];
    int nameLength;
    int constValLength;
    int minValLength;
    int maxValLength;
    int nullValLength;
    int characterEncodingLength;
    int epochLength;
    int timeUnitLength;
    int semanticTypeLength;
    uint32_t serializedTokenSize;
};

#if !defined(WIN32)
const int Ir::INVALID_ID;
const uint32_t Ir::VARIABLE_SIZE;
#endif /* WIN32 */

Ir::Ir(const char *buffer, const int len, const int64_t templateId, const int64_t templateVersion) :
    buffer_(buffer), len_(len), templateId_(templateId), templateVersion_(templateVersion)
{
    impl_ = new Ir::Impl;
    begin();
}

Ir::~Ir()
{
    if (impl_ != NULL)
    {
        delete impl_;
        impl_ = NULL;
    }
//    std::cout << "Ir being deleted" << "\n";
}

void Ir::readTokenAtCurrentPosition()
{
    char tmp[256];

    //printf("read buffer_ %p offset %d\n", buffer_, cursorOffset_);

    impl_->tokenCodec.wrapForDecode((char *)buffer_, cursorOffset_,
        impl_->tokenCodec.blockLength(), impl_->tokenCodec.templateVersion());

    // read all the var data and save in Impl then save size

    impl_->nameLength = impl_->tokenCodec.getName(impl_->name, sizeof(impl_->name));

    impl_->constValLength = impl_->tokenCodec.getConstVal(impl_->constVal, sizeof(impl_->constVal));

    // don't really do anything with min/max/null/encoding right now
    impl_->minValLength = impl_->tokenCodec.getMinVal(impl_->minVal, sizeof(tmp));
    impl_->maxValLength = impl_->tokenCodec.getMaxVal(impl_->maxVal, sizeof(tmp));
    impl_->nullValLength = impl_->tokenCodec.getNullVal(impl_->nullVal, sizeof(tmp));
    impl_->characterEncodingLength = impl_->tokenCodec.getCharacterEncoding(impl_->characterEncoding, sizeof(tmp));
    impl_->epochLength = impl_->tokenCodec.getEpoch(impl_->epoch, sizeof(tmp));
    impl_->timeUnitLength = impl_->tokenCodec.getTimeUnit(impl_->timeUnit, sizeof(tmp));
    impl_->semanticTypeLength = impl_->tokenCodec.getSemanticType(impl_->semanticType, sizeof(tmp));

    impl_->serializedTokenSize = impl_->tokenCodec.size();

//    printf("token %p %d offset=%d size=%d id=%d signal=%d type=%d order=%d name=%s constLen=%d\n",
//           buffer_, cursorOffset_, offset(), size(), schemaId(), signal(), primitiveType(), byteOrder(),
//           name().c_str(), impl_->constValLength);

}

void Ir::begin()
{
    cursorOffset_ = 0;

    if (buffer_ != NULL)
    {
        readTokenAtCurrentPosition();
    }
}

void Ir::next()
{
    cursorOffset_ += impl_->serializedTokenSize;

    if (!end())
    {
        readTokenAtCurrentPosition();
    }
}

bool Ir::end() const
{
    if (cursorOffset_ < len_)
    {
        return false;
    }
    return true;
}

int32_t Ir::offset() const
{
    return impl_->tokenCodec.tokenOffset();
}

int32_t Ir::size() const
{
    return impl_->tokenCodec.tokenSize();
}

Ir::TokenSignal Ir::signal() const
{
    // the serialized IR and the Ir::TokenSignal enums MUST be kept in sync!
    return (Ir::TokenSignal)impl_->tokenCodec.signal();
}

Ir::TokenByteOrder Ir::byteOrder() const
{
    // the serialized IR and the Ir::TokenByteOrder enums MUST be kept in sync!
    return (Ir::TokenByteOrder)impl_->tokenCodec.byteOrder();
}

Ir::TokenPrimitiveType Ir::primitiveType() const
{
    // the serialized IR and the Ir::TokenPrimitiveType enums MUST be kept in sync!
    return (Ir::TokenPrimitiveType)impl_->tokenCodec.primitiveType();
}

Ir::TokenPresence Ir::presence() const
{
    return (Ir::TokenPresence)impl_->tokenCodec.presence();
}

int32_t Ir::schemaId() const
{
    return impl_->tokenCodec.schemaId();
}

uint64_t Ir::validValue() const
{
    // constVal holds the validValue. primitiveType holds the type
    switch (primitiveType())
    {
        case Ir::CHAR:
            return impl_->constVal[0];
            break;

        case Ir::UINT8:
            return impl_->constVal[0];
            break;

        default:
            throw "do not know validValue primitiveType";
            break;
    }
}

uint64_t Ir::choiceValue() const
{
    // constVal holds the validValue. primitiveType holds the type
    switch (primitiveType())
    {
        case Ir::UINT8:
            return impl_->constVal[0];
            break;

        case Ir::UINT16:
            return *(uint16_t *)(impl_->constVal);
            break;

        case Ir::UINT32:
            return *(uint32_t *)(impl_->constVal);
            break;

        case Ir::UINT64:
            return *(uint64_t *)(impl_->constVal);
            break;

        default:
            throw "do not know choice primitiveType";
            break;
    }
}

int64_t Ir::nameLen() const
{
    return impl_->nameLength;
}

std::string Ir::name() const
{
    return std::string(impl_->name, nameLen());
}

int64_t Ir::constLen() const
{
    return impl_->constValLength;
}

const char *Ir::constVal() const
{
    if (constLen() == 0)
    {
        return NULL;
    }

    return impl_->constVal;
}

int64_t Ir::minLen() const
{
    return impl_->minValLength;
}

const char *Ir::minVal() const
{
    if (minLen() == 0)
    {
        return NULL;
    }

    return impl_->minVal;
}

int64_t Ir::maxLen() const
{
    return impl_->maxValLength;
}

const char *Ir::maxVal() const
{
    if (maxLen() == 0)
    {
        return NULL;
    }

    return impl_->maxVal;
}

int64_t Ir::nullLen() const
{
    return impl_->nullValLength;
}

const char *Ir::nullVal() const
{
    if (nullLen() == 0)
    {
        return NULL;
    }

    return impl_->nullVal;
}

int64_t Ir::characterEncodingLen() const
{
    return impl_->characterEncodingLength;
}

const char *Ir::characterEncoding() const
{
    if (characterEncodingLen() == 0)
    {
        return NULL;
    }

    return impl_->characterEncoding;
}

int64_t Ir::epochLen() const
{
    return impl_->epochLength;
}

const char *Ir::epoch() const
{
    if (epochLen() == 0)
    {
        return NULL;
    }

    return impl_->epoch;
}

int64_t Ir::timeUnitLen() const
{
    return impl_->timeUnitLength;
}

const char *Ir::timeUnit() const
{
    if (timeUnitLen() == 0)
    {
        return NULL;
    }

    return impl_->timeUnit;
}

int64_t Ir::semanticTypeLen() const
{
    return impl_->semanticTypeLength;
}

const char *Ir::semanticType() const
{
    if (semanticTypeLen() == 0)
    {
        return NULL;
    }

    return impl_->semanticType;
}

int Ir::position() const
{
    return cursorOffset_;
}

void Ir::position(const int pos)
{
    cursorOffset_ = pos;
    readTokenAtCurrentPosition();
}

void Ir::addToken(uint32_t offset,
                  uint32_t size,
                  TokenSignal signal,
                  TokenByteOrder byteOrder,
                  TokenPrimitiveType primitiveType,
                  uint16_t schemaId,
                  const std::string &name,
                  const char *constVal,
                  int constValLength)
{
    TokenCodec tokenCodec;

    if (buffer_ == NULL)
    {
        buffer_ = new char[4098];
    }

    //printf("buffer_ %p offset %d\n", buffer_, cursorOffset_);

    tokenCodec.wrapForEncode((char *)buffer_, cursorOffset_);

    tokenCodec.tokenOffset(offset)
              .tokenSize(size)
              .schemaId(schemaId)
              .tokenVersion(0)
              .signal((SignalCodec::Value)signal)
              .primitiveType((PrimitiveTypeCodec::Value)primitiveType)
              .byteOrder((ByteOrderCodec::Value)byteOrder);

    tokenCodec.putName(name.c_str(), name.size());
    tokenCodec.putConstVal(constVal, constValLength);
    tokenCodec.putMinVal(NULL, 0);
    tokenCodec.putMaxVal(NULL, 0);
    tokenCodec.putNullVal(NULL, 0);
    tokenCodec.putCharacterEncoding(NULL, 0);
    tokenCodec.putEpoch(NULL, 0);
    tokenCodec.putTimeUnit(NULL, 0);
    tokenCodec.putSemanticType(NULL, 0);

    cursorOffset_ += tokenCodec.size();
    len_ = cursorOffset_;
}
