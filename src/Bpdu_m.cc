//
// Generated file, do not edit! Created by opp_msgtool 6.5 from Bpdu.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "Bpdu_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// std::vector<bool> elements are proxy objects (also for const access in libc++), so pack/unpack via a temporary
template<typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<bool,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++) {
        bool x = v[i];
        doParsimPacking(buffer, x);
    }
}

template<typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<bool,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++) {
        bool x;
        doParsimUnpacking(buffer, x);
        v[i] = x;
    }
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// Returns the string form of t produced by operator<<, or the given fallback string if there is no operator<<.
// (The fallback should be "" for compound types, whose fields can be displayed, and "(unprintable)" otherwise.)
template<typename T>
std::string toStringIfPrintable(const T& t, const char *fallback) {
    if constexpr (omnetpp::internal::is_printable<T>::value) {
        std::ostringstream os;
        os << t;
        return os.str();
    }
    return fallback;
}

template<typename T>
bool fromStringIfExtractable(T& t, const char *s) {
    if constexpr (omnetpp::internal::is_extractable<T>::value) {
        std::istringstream is(s);
        is >> t;
        return true;
    }
    return false;
}

Register_Class(Bpdu)

Bpdu::Bpdu(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

Bpdu::Bpdu(const Bpdu& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

Bpdu::~Bpdu()
{
}

Bpdu& Bpdu::operator=(const Bpdu& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void Bpdu::copy(const Bpdu& other)
{
    this->rootId = other.rootId;
    this->rootPriority = other.rootPriority;
    this->rootPathCost = other.rootPathCost;
    this->bridgeId = other.bridgeId;
    this->bridgePriority = other.bridgePriority;
    this->portId = other.portId;
    this->validUntil = other.validUntil;
}

void Bpdu::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->rootId);
    doParsimPacking(b,this->rootPriority);
    doParsimPacking(b,this->rootPathCost);
    doParsimPacking(b,this->bridgeId);
    doParsimPacking(b,this->bridgePriority);
    doParsimPacking(b,this->portId);
    doParsimPacking(b,this->validUntil);
}

void Bpdu::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->rootId);
    doParsimUnpacking(b,this->rootPriority);
    doParsimUnpacking(b,this->rootPathCost);
    doParsimUnpacking(b,this->bridgeId);
    doParsimUnpacking(b,this->bridgePriority);
    doParsimUnpacking(b,this->portId);
    doParsimUnpacking(b,this->validUntil);
}

int64_t Bpdu::getRootId() const
{
    return this->rootId;
}

void Bpdu::setRootId(int64_t rootId)
{
    this->rootId = rootId;
}

int64_t Bpdu::getRootPriority() const
{
    return this->rootPriority;
}

void Bpdu::setRootPriority(int64_t rootPriority)
{
    this->rootPriority = rootPriority;
}

int64_t Bpdu::getRootPathCost() const
{
    return this->rootPathCost;
}

void Bpdu::setRootPathCost(int64_t rootPathCost)
{
    this->rootPathCost = rootPathCost;
}

int64_t Bpdu::getBridgeId() const
{
    return this->bridgeId;
}

void Bpdu::setBridgeId(int64_t bridgeId)
{
    this->bridgeId = bridgeId;
}

int64_t Bpdu::getBridgePriority() const
{
    return this->bridgePriority;
}

void Bpdu::setBridgePriority(int64_t bridgePriority)
{
    this->bridgePriority = bridgePriority;
}

int Bpdu::getPortId() const
{
    return this->portId;
}

void Bpdu::setPortId(int portId)
{
    this->portId = portId;
}

omnetpp::simtime_t Bpdu::getValidUntil() const
{
    return this->validUntil;
}

void Bpdu::setValidUntil(omnetpp::simtime_t validUntil)
{
    this->validUntil = validUntil;
}

class BpduDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_rootId,
        FIELD_rootPriority,
        FIELD_rootPathCost,
        FIELD_bridgeId,
        FIELD_bridgePriority,
        FIELD_portId,
        FIELD_validUntil,
    };
  public:
    BpduDescriptor();
    virtual ~BpduDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(BpduDescriptor)

BpduDescriptor::BpduDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(Bpdu)), "omnetpp::cMessage")
{
    propertyNames = nullptr;
}

BpduDescriptor::~BpduDescriptor()
{
    delete[] propertyNames;
}

bool BpduDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Bpdu *>(obj)!=nullptr;
}

const char **BpduDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *BpduDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string BpduDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void BpduDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int BpduDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 7+base->getFieldCount() : 7;
}

unsigned int BpduDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_rootId
        FD_ISEDITABLE,    // FIELD_rootPriority
        FD_ISEDITABLE,    // FIELD_rootPathCost
        FD_ISEDITABLE,    // FIELD_bridgeId
        FD_ISEDITABLE,    // FIELD_bridgePriority
        FD_ISEDITABLE,    // FIELD_portId
        FD_ISEDITABLE,    // FIELD_validUntil
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *BpduDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "rootId",
        "rootPriority",
        "rootPathCost",
        "bridgeId",
        "bridgePriority",
        "portId",
        "validUntil",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int BpduDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "rootId") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "rootPriority") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "rootPathCost") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "bridgeId") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "bridgePriority") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "portId") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "validUntil") == 0) return baseIndex + 6;
    return base ? base->findField(fieldName) : -1;
}

const char *BpduDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int64_t",    // FIELD_rootId
        "int64_t",    // FIELD_rootPriority
        "int64_t",    // FIELD_rootPathCost
        "int64_t",    // FIELD_bridgeId
        "int64_t",    // FIELD_bridgePriority
        "int",    // FIELD_portId
        "omnetpp::simtime_t",    // FIELD_validUntil
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **BpduDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *BpduDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int BpduDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void BpduDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'Bpdu'", field);
    }
}

const char *BpduDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string BpduDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        case FIELD_rootId: return int642string(pp->getRootId());
        case FIELD_rootPriority: return int642string(pp->getRootPriority());
        case FIELD_rootPathCost: return int642string(pp->getRootPathCost());
        case FIELD_bridgeId: return int642string(pp->getBridgeId());
        case FIELD_bridgePriority: return int642string(pp->getBridgePriority());
        case FIELD_portId: return long2string(pp->getPortId());
        case FIELD_validUntil: return simtime2string(pp->getValidUntil());
        default: return "";
    }
}

void BpduDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        case FIELD_rootId: pp->setRootId(string2int64(value)); break;
        case FIELD_rootPriority: pp->setRootPriority(string2int64(value)); break;
        case FIELD_rootPathCost: pp->setRootPathCost(string2int64(value)); break;
        case FIELD_bridgeId: pp->setBridgeId(string2int64(value)); break;
        case FIELD_bridgePriority: pp->setBridgePriority(string2int64(value)); break;
        case FIELD_portId: pp->setPortId(string2long(value)); break;
        case FIELD_validUntil: pp->setValidUntil(string2simtime(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'Bpdu'", field);
    }
}

omnetpp::cValue BpduDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        case FIELD_rootId: return pp->getRootId();
        case FIELD_rootPriority: return pp->getRootPriority();
        case FIELD_rootPathCost: return pp->getRootPathCost();
        case FIELD_bridgeId: return pp->getBridgeId();
        case FIELD_bridgePriority: return pp->getBridgePriority();
        case FIELD_portId: return pp->getPortId();
        case FIELD_validUntil: return pp->getValidUntil().dbl();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'Bpdu' as cValue -- field index out of range?", field);
    }
}

void BpduDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        case FIELD_rootId: pp->setRootId(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_rootPriority: pp->setRootPriority(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_rootPathCost: pp->setRootPathCost(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_bridgeId: pp->setBridgeId(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_bridgePriority: pp->setBridgePriority(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_portId: pp->setPortId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_validUntil: pp->setValidUntil(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'Bpdu'", field);
    }
}

const char *BpduDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr BpduDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void BpduDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    Bpdu *pp = omnetpp::fromAnyPtr<Bpdu>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'Bpdu'", field);
    }
}

Register_Class(StpProbe)

StpProbe::StpProbe(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

StpProbe::StpProbe(const StpProbe& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

StpProbe::~StpProbe()
{
}

StpProbe& StpProbe::operator=(const StpProbe& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void StpProbe::copy(const StpProbe& other)
{
    this->source = other.source;
    this->destination = other.destination;
    this->sequence = other.sequence;
    this->hopsLeft = other.hopsLeft;
}

void StpProbe::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->source);
    doParsimPacking(b,this->destination);
    doParsimPacking(b,this->sequence);
    doParsimPacking(b,this->hopsLeft);
}

void StpProbe::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->source);
    doParsimUnpacking(b,this->destination);
    doParsimUnpacking(b,this->sequence);
    doParsimUnpacking(b,this->hopsLeft);
}

int64_t StpProbe::getSource() const
{
    return this->source;
}

void StpProbe::setSource(int64_t source)
{
    this->source = source;
}

int64_t StpProbe::getDestination() const
{
    return this->destination;
}

void StpProbe::setDestination(int64_t destination)
{
    this->destination = destination;
}

int64_t StpProbe::getSequence() const
{
    return this->sequence;
}

void StpProbe::setSequence(int64_t sequence)
{
    this->sequence = sequence;
}

int StpProbe::getHopsLeft() const
{
    return this->hopsLeft;
}

void StpProbe::setHopsLeft(int hopsLeft)
{
    this->hopsLeft = hopsLeft;
}

class StpProbeDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_source,
        FIELD_destination,
        FIELD_sequence,
        FIELD_hopsLeft,
    };
  public:
    StpProbeDescriptor();
    virtual ~StpProbeDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(StpProbeDescriptor)

StpProbeDescriptor::StpProbeDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(StpProbe)), "omnetpp::cMessage")
{
    propertyNames = nullptr;
}

StpProbeDescriptor::~StpProbeDescriptor()
{
    delete[] propertyNames;
}

bool StpProbeDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<StpProbe *>(obj)!=nullptr;
}

const char **StpProbeDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *StpProbeDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string StpProbeDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void StpProbeDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int StpProbeDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 4+base->getFieldCount() : 4;
}

unsigned int StpProbeDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_source
        FD_ISEDITABLE,    // FIELD_destination
        FD_ISEDITABLE,    // FIELD_sequence
        FD_ISEDITABLE,    // FIELD_hopsLeft
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *StpProbeDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "source",
        "destination",
        "sequence",
        "hopsLeft",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int StpProbeDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "source") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "destination") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "sequence") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "hopsLeft") == 0) return baseIndex + 3;
    return base ? base->findField(fieldName) : -1;
}

const char *StpProbeDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int64_t",    // FIELD_source
        "int64_t",    // FIELD_destination
        "int64_t",    // FIELD_sequence
        "int",    // FIELD_hopsLeft
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **StpProbeDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *StpProbeDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int StpProbeDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void StpProbeDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'StpProbe'", field);
    }
}

const char *StpProbeDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string StpProbeDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        case FIELD_source: return int642string(pp->getSource());
        case FIELD_destination: return int642string(pp->getDestination());
        case FIELD_sequence: return int642string(pp->getSequence());
        case FIELD_hopsLeft: return long2string(pp->getHopsLeft());
        default: return "";
    }
}

void StpProbeDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        case FIELD_source: pp->setSource(string2int64(value)); break;
        case FIELD_destination: pp->setDestination(string2int64(value)); break;
        case FIELD_sequence: pp->setSequence(string2int64(value)); break;
        case FIELD_hopsLeft: pp->setHopsLeft(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'StpProbe'", field);
    }
}

omnetpp::cValue StpProbeDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        case FIELD_source: return pp->getSource();
        case FIELD_destination: return pp->getDestination();
        case FIELD_sequence: return pp->getSequence();
        case FIELD_hopsLeft: return pp->getHopsLeft();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'StpProbe' as cValue -- field index out of range?", field);
    }
}

void StpProbeDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        case FIELD_source: pp->setSource(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_destination: pp->setDestination(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_sequence: pp->setSequence(omnetpp::checked_int_cast<int64_t>(value.intValue())); break;
        case FIELD_hopsLeft: pp->setHopsLeft(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'StpProbe'", field);
    }
}

const char *StpProbeDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr StpProbeDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void StpProbeDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    StpProbe *pp = omnetpp::fromAnyPtr<StpProbe>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'StpProbe'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp

