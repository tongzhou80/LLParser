//
// Created by tlaber on 6/15/17.
//

#include <asmParser/sysDict.h>
#include "diEssential.h"

DILocation::DILocation() {
    _line = 0;
    _column = 0;
    _inline_root = NULL;
    _subprogram = NULL;
    _scope = NULL;
    _inlinedAt = NULL;
}

void DILocation::resolve() {
    /* resolve direct symbols */
    DI_SET_INT_FIELD(line);
    DI_SET_INT_FIELD(column);
    DI_SET_REF_FIELD(scope, DIScope);
    DI_SET_REF_FIELD(inlinedAt, DILocation);

    set_is_resolved();
}

void DILocation::second_resolve() {
    if (_number == 42531) {
        zpl("42531")
    }
    /* resolve indirect symbols */
    for (_inline_root = inlinedAt(); _inline_root; ) {
        // todo: verify that the referenced inlined location should already be parsed at this point
        _inline_root = _inline_root->inlinedAt();
    }

    DIScope* info = _scope;


    //zpl("info: %p, scope: %p", info, _scope);

    if (_inline_root) {
        info = _inline_root->scope();
    }

    DISubprogram* subp = dynamic_cast<DISubprogram*>(info);
    //zpl("raw: %s", raw_c_str())
    /* if scope is not of type DISubprogram, check if it is other types */
    while (!subp) {
        if (DILexicalBlock* b = dynamic_cast<DILexicalBlock*>(info)) {
            //zpl("%d", b->scope_id());
            //info = SysDict::module()->get_debug_info(b->scope_id());
            info = b->scope();
            subp = dynamic_cast<DISubprogram*>(info);
        }
        else if (DILexicalBlockFile* b = dynamic_cast<DILexicalBlockFile*>(info)) {
            //zpl("%d", b->scope_id());
            info = b->scope();
            subp = dynamic_cast<DISubprogram*>(info);
        }
        else {
            guarantee(0, " ");
        }
    }
    _subprogram = subp;
}

///* _orig might still be NULL after resolve, but _subprogram should not be NULL */
//void DILocation::resolve() {
//    int orig_scope = _scope_id;
//    int inlined_pos = _inlined_at;
//    while (inlined_pos > 0) {
//        MetaData* orig = SysDict::module()->get_debug_info(inlined_pos);
//        _orig = dynamic_cast<DILocation*>(orig);
//        guarantee(_orig != NULL, "must be a DILocation");
//        orig_scope = _orig->scope_id();
//        inlined_pos = _orig->inlined_pos();
//    }
//
//
//    MetaData* info = (DISubprogram*)SysDict::module()->get_debug_info(orig_scope);
//    DISubprogram* subp = dynamic_cast<DISubprogram*>(info);
//    while (!subp) {
//        if (DILexicalBlock* b = dynamic_cast<DILexicalBlock*>(info)) {
//            //zpl("%d", b->scope_id());
//            info = SysDict::module()->get_debug_info(b->scope_id());
//            subp = dynamic_cast<DISubprogram*>(info);
//        }
//        else if (DILexicalBlockFile* b = dynamic_cast<DILexicalBlockFile*>(info)) {
//            //zpl("%d", b->scope_id());
//            info = SysDict::module()->get_debug_info(b->scope_id());
//            subp = dynamic_cast<DISubprogram*>(info);
//        }
//        else {
//            guarantee(0, " ");
//        }
//    }
//    _subprogram = subp;
//}

int DILocation::line() {
    return _line;
//    if (_inline_root) {
//        return _inline_root->line();
//    }
//    else {
//        return _line;
//    }
}

string DILocation::filename() {
    return _scope->filename();
//    DISubprogram*& p = _subprogram;
//    if (p == NULL) {
//        return "";
//    }
//    else {
//        return p->filename();
//    }
}
//
//DISubprogram* DILocation::get_subprogram() {
//    int orig_scope = _scope_id;
//    int inlined_pos = _inlined_at;
//    while (inlined_pos > 0) {
//        MetaData* orig = SysDict::module()->get_debug_info(inlined_pos);
//        zpl("inlined: %d", inlined_pos);
//        DILocation* _orig = dynamic_cast<DILocation*>(orig);
//        guarantee(_orig != NULL, "must be a DILocation");
//        orig_scope = _orig->scope_id();
//        inlined_pos = _orig->inlined_pos();
//    }
//
//
//    MetaData* info = (DISubprogram*)SysDict::module()->get_debug_info(orig_scope);
//    DISubprogram* subp = dynamic_cast<DISubprogram*>(info);
//    while (!subp) {
//        if (DILexicalBlock* b = dynamic_cast<DILexicalBlock*>(info)) {
//            //zpl("%d", b->scope_id());
//            info = SysDict::module()->get_debug_info(b->scope_id());
//            subp = dynamic_cast<DISubprogram*>(info);
//        }
//        else if (DILexicalBlockFile* b = dynamic_cast<DILexicalBlockFile*>(info)) {
//            //zpl("%d", b->scope_id());
//            info = SysDict::module()->get_debug_info(b->scope_id());
//            subp = dynamic_cast<DISubprogram*>(info);
//        }
//        else {
//            guarantee(0, " ");
//        }
//    }
//    _subprogram = subp;
//}

string DILocation::function() {
    guarantee(_subprogram != NULL, "An instruction should have a function scope");
    return _subprogram->name();
}

string DILocation::function_name() {
    guarantee(_subprogram != NULL, "An instruction should have a function scope");
    return _subprogram->name();
}

string DILocation::function_linkage_name() {
    guarantee(_subprogram != NULL, "An instruction should have a function scope");
    if (_subprogram->linkageName().empty()) {
        return _subprogram->name();
    }
    else {
        return _subprogram->linkageName();
    }
}

string DILocation::dump(bool newline) {
    string ss = this->filename() + ':' + std::to_string((long long)this->line()) + " (" + this->function().c_str() + ')';
    printf("%s", ss.c_str());
    if (newline) {
        printf("\n");
    }
    return ss;
}