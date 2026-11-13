#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only section generator: copy the actual fixture's dependency closure, not data."""
import ast
import inspect
import types


class Specialize(ast.NodeTransformer):
    def __init__(self,name): self.name=name
    def choice(self,test):
        if (isinstance(test,ast.Compare) and isinstance(test.left,ast.Name)
                and test.left.id=='name' and len(test.ops)==1
                and isinstance(test.ops[0],ast.Eq) and isinstance(test.comparators[0],ast.Constant)):
            return self.name==test.comparators[0].value
        return None
    def visit_If(self,node):
        node=self.generic_visit(node)
        selected=self.choice(node.test)
        return node if selected is None else node.body if selected else node.orelse
    def visit_IfExp(self,node):
        node=self.generic_visit(node)
        selected=self.choice(node.test)
        return node if selected is None else node.body if selected else node.orelse


def generate(name,factory,parameters,controls,runtime):
    definitions={}; constants={}; visiting=set()
    imports={'base64','struct','Decimal','localcontext','Fraction','lru_cache'}
    def visit(symbol,value):
        if symbol in imports: return
        fn=inspect.unwrap(value)
        if not inspect.isfunction(fn):
            assert not isinstance(value,types.ModuleType),f'unexpected fixture import {symbol}'
            literal=repr(value)
            assert symbol not in constants or constants[symbol]==literal,f'constant conflict {symbol}'
            constants[symbol]=literal
            return
        assert fn.__name__==symbol and not fn.__code__.co_freevars,f'fixture closure {symbol}'
        tree=ast.parse(inspect.getsource(fn))
        if hasattr(value,'cache_info') and not tree.body[0].decorator_list:
            tree.body[0].decorator_list=[ast.parse('lru_cache(None)',mode='eval').body]
        text=ast.unparse(tree)
        if symbol in definitions:
            assert definitions[symbol]==text,f'function conflict {symbol}'
            return
        assert symbol not in visiting,f'recursive fixture {symbol}'
        visiting.add(symbol)
        pending=[fn.__code__]
        while pending:
            code=pending.pop()
            for dependency in code.co_names:
                if dependency in fn.__globals__: visit(dependency,fn.__globals__[dependency])
            pending.extend(value for value in code.co_consts if isinstance(value,types.CodeType))
        visiting.remove(symbol)
        definitions[symbol]=text
    visit(factory.__name__,factory)
    header='import base64,struct\nfrom decimal import Decimal,localcontext\nfrom fractions import Fraction\nfrom functools import lru_cache\n'
    assignments='\n'.join(key+'='+value for key,value in sorted(constants.items()))
    create=('def _make_payload():\n'
            f'    cases=[{factory.__name__}(*args) for args in {tuple(parameters)!r}]\n'
            f'    return dict(name={name!r},cases=cases,controls={list(controls)!r},timing=len(cases)-1)\n')
    runtime=ast.unparse(Specialize(name).visit(ast.parse(runtime)))
    return '# Generated from canonical host fixture/oracle functions; no embedded memory images.\n'+header+assignments+'\n'+'\n'.join(definitions.values())+'\n'+create+runtime+'\n'


def fresh_namespace(source,name):
    """Match the loader's exec(source,{}) and check deferred function lookups too."""
    import builtins
    import dis
    namespace={}
    exec(compile(source,name,'exec'),namespace)
    functions=[inspect.unwrap(value) for value in namespace.values()]
    pending=[value.__code__ for value in functions if inspect.isfunction(value) and value.__globals__ is namespace]
    seen=set()
    while pending:
        code=pending.pop()
        if code in seen: continue
        seen.add(code)
        for instruction in dis.get_instructions(code):
            if instruction.opname=='LOAD_GLOBAL':
                symbol=instruction.argval
                assert symbol in namespace or hasattr(builtins,symbol), f'unresolved notebook global {symbol} in {code.co_name}'
        pending.extend(value for value in code.co_consts if isinstance(value,types.CodeType))
    return namespace
