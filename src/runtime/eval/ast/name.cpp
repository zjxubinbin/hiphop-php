/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/base/complex_types.h>
#include <runtime/eval/analysis/block.h>
#include <util/util.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

Name::Name(CONSTRUCT_ARGS) : Construct(CONSTRUCT_PASS) {}

int64 Name::hash() const {
  return -1;
}

bool Name::isSp() const { return false; }

NamePtr Name::fromString(CONSTRUCT_ARGS, const string &name,
                         bool isSp /* = false */) {
  return NamePtr(new StringName(CONSTRUCT_PASS, name, isSp));
}

NamePtr Name::fromString(CONSTRUCT_ARGS, CStrRef name,
                         bool isSp /* = false */) {
  return NamePtr(new StringName(CONSTRUCT_PASS, name, isSp));
}

NamePtr Name::fromExp(CONSTRUCT_ARGS, ExpressionPtr e) {
  return NamePtr(new ExprName(CONSTRUCT_PASS, e));
}
NamePtr Name::fromStaticClassExp(CONSTRUCT_ARGS, ExpressionPtr e) {
  return NamePtr(new StaticClassExprName(CONSTRUCT_PASS, e));
}
NamePtr Name::LateStatic(CONSTRUCT_ARGS) {
  return NamePtr(new LateStaticName(CONSTRUCT_PASS));
}

StringName::StringName(CONSTRUCT_ARGS, const string &name,
                       bool isSp /* = false */)
  : Name(CONSTRUCT_PASS), m_name(StringData::GetStaticString(name)),
  m_isSp(isSp), m_sg(VariableIndex::isSuperGlobal(m_name)) {
}

StringName::StringName(CONSTRUCT_ARGS, CStrRef name, bool isSp /* = false */)
  : Name(CONSTRUCT_PASS),
  m_name(name.get()), m_isSp(isSp), m_sg(VariableIndex::isSuperGlobal(m_name)) {
  ASSERT(m_name->isStatic());
}

bool StringName::getSuperGlobal(SuperGlobal &sg) {
  sg = m_sg;
  return true;
}

String StringName::get(VariableEnvironment &env) const {
  return m_name;
}

String StringName::get() const {
  return m_name;
}

int64 StringName::hash() const {
  return m_name->hash();
}

bool StringName::isSp() const { return m_isSp; }

void StringName::dump(std::ostream &out) const {
  const std::string &originalText = getOriginalText();
  if (!originalText.empty()) {
    out << originalText;
  } else {
    out << m_name->data();
  }
}

ExprName::ExprName(CONSTRUCT_ARGS, ExpressionPtr name)
  : Name(CONSTRUCT_PASS), m_name(name) {}

Variant ExprName::getAsVariant(VariableEnvironment &env) const {
  return m_name->eval(env);
}

void ExprName::dump(std::ostream &out) const {
  if (m_name) {
    m_name->dump(out);
  }
}

void ExprName::setExp(ExpressionPtr name) {
  m_name = name;
}

Name *ExprName::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_name);
  return NULL;
}

StaticClassExprName::StaticClassExprName(CONSTRUCT_ARGS, ExpressionPtr name)
  : ExprName(CONSTRUCT_PASS, name) {}

String StaticClassExprName::get(VariableEnvironment &env) const {
  return get_static_class_name(m_name->eval(env));
}

LateStaticName::LateStaticName(CONSTRUCT_ARGS) : Name(CONSTRUCT_PASS) {}

String LateStaticName::get(VariableEnvironment &env) const {
  return FrameInjection::GetStaticClassName(
    ThreadInfo::s_threadInfo.getNoCheck());
}

void LateStaticName::dump(std::ostream &out) const {
  out << "static";
}

void optimize(VariableEnvironment &env, NamePtr &before) {
  if (before) {
    Name *optName = before->optimize(env);
    if (optName) {
      NamePtr after = dynamic_cast<Name*>(optName);
      if (after) {
        before = after;
      } else {
        assert(false);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

