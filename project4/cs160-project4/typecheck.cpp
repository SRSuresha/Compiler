#include <iostream>
#include <cstdio>
#include <cstring>

#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"

// WRITEME: The default attribute propagation rule
#define default_rule(X) X

#include <typeinfo>

class Typecheck : public Visitor
{
  private:
    FILE* m_errorfile;
    SymTab* m_st;

    // The set of recognized errors
    enum errortype
    {
        no_main,
        nonvoid_main,
        dup_proc_name,
        dup_var_name,
        proc_undef,
        call_type_mismatch,
        narg_mismatch,
        expr_type_err,
        var_undef,
        proc_as_var,
        ifpred_err,
        whilepred_err,
        incompat_assign,
        who_knows,
        ret_type_mismatch,
        array_index_error,
        no_array_var,
        arg_type_mismatch,
        expr_pointer_arithmetic_err,
        expr_abs_error,
        expr_addressof_error,
        invalid_deref
    };

    // Print the error to file and exit
    void t_error(errortype e, Attribute a)
    {
        fprintf(m_errorfile,"on line number %d, ", a.lineno);

        switch(e)
        {
            case no_main:
                fprintf(m_errorfile, "error: no main\n");
                break;
            case dup_proc_name:
                fprintf(m_errorfile, "error: duplicate procedure names in same scope\n");
                break;
            case nonvoid_main:
                fprintf(m_errorfile, "error: the Main procedure has arguments\n");
                break;
            case dup_var_name:
                fprintf(m_errorfile, "error: duplicate variable names in same scope\n");
                break;
            case proc_undef:
                fprintf(m_errorfile, "error: call to undefined procedure\n");
                break;
            case narg_mismatch:
                fprintf(m_errorfile, "error: procedure call has different number of args than declartion\n");
                break;
            case expr_type_err:
                fprintf(m_errorfile, "error: incompatible types used in expression\n");
                break;
            case var_undef:
                fprintf(m_errorfile, "error: undefined variable\n");
                break;
            case proc_as_var:
                fprintf(m_errorfile, "error: tried to use a prodecure as a variable\n");
                break;
            case ifpred_err:
                fprintf(m_errorfile, "error: predicate of if statement is not boolean\n");
                break;
            case whilepred_err:
                fprintf(m_errorfile, "error: predicate of while statement is not boolean\n");
                break;
            case incompat_assign:
                fprintf(m_errorfile, "error: type of expr and var do not match in assignment\n");
                break;
            case call_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in procedure call args\n");
                break;
            case ret_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in return statement\n");
                break;
            case array_index_error:
                fprintf(m_errorfile, "error: array index not integer\n");
                break;
            case no_array_var:
                fprintf(m_errorfile, "error: attempt to index non-array variable\n");
                break;
            case arg_type_mismatch:
                fprintf(m_errorfile, "error: argument type mismatch\n");
                break;
            case expr_pointer_arithmetic_err:
                fprintf(m_errorfile, "error: invalid pointer arithmetic\n");
                break;
            case expr_abs_error:
                fprintf(m_errorfile, "error: absolute value can only be applied to integers and strings\n");
                break;
            case expr_addressof_error:
                fprintf(m_errorfile, "error: AddressOf can only be applied to integers, chars, and indexed strings\n");
                break;
            case invalid_deref:
                fprintf(m_errorfile, "error: Deref can only be applied to integer pointers and char pointers\n");
                break;
            default:
                fprintf(m_errorfile, "error: no good reason\n");
                break;
        }
        exit(1);
    }


    // Check that there is one and only one main
    void check_for_one_main(ProgramImpl* p)
    {
        std::list<Proc_ptr>::iterator iter;
        char * name;
        char target_name[] = "Main";
        char number_of_main = 0;

        for(iter = p->m_proc_list->begin(); iter != p->m_proc_list->end(); ++iter){
            name = strdup(((ProcImpl*)(*iter))->m_symname->spelling());
            if(strcmp(target_name,name) == 0)
                number_of_main++;
        }

        if(number_of_main!=1){
            this->t_error(no_main,p->m_attribute);
        }

    }

    // Create a symbol for the procedure and check there is none already
    // existing
    void add_proc_symbol(ProcImpl* p)
    {
        std::list<Decl_ptr>::iterator iter;
        char * name; Symbol *s = new Symbol();
        name =strdup(p->m_symname->spelling());
        for (iter = p->m_decl_list->begin(); iter != p->m_decl_list->end(); ++iter){
            add_decl_symbol(*iter);
            s.m_arg_type.push_back(*iter->m_type->m_attribute.m_basetype);
        }
        s->m_basetype = bt_procedure;
        if(!m_st->insert(name,s))
            this->t_error(dup_proc_name, p->m_attribute);
    }

    // Add symbol table information for all the declarations following
    void add_decl_symbol(DeclImpl* p)
    {
        std::list<SymName_ptr>::iterator iter;
        char * name; Symbol *s;
        for(iter = p->m_symname_list->begin(); iter !=p->m_symname_list()->end(), ++iter){
            name = strdup((*iter)->spelling());
            s = new Symbol();
            s.m_basetype = p->m_type->m_attribute.m_basetype;

            if(! m_st->insert(name,s)){
                this->t_error(dup_var_name, p->m_attribute);
            }
        }
    }

    // Check that the return statement of a procedure has the appropriate type
    void check_proc(ProcImpl *p)
    {
        Basetype corrrect_type, real_type;
        corrrect_type = p->m_type->m_attribute.m_basetype;
        real_type = p->m_procedure_block->m_return_stat->m_expr->m_attribute.m_basetype;
        if(corrrect_type!=real_type){
            this->t_error(ret_type_mismatch,p->m_attribute);
        }
    }

    // Check that the declared return type is not an array
    void check_return(Return *p)
    {
        Basetype return_type;
        return_type = p->m_expr->m_attribute.m_basetype;
        if(return_type == bt_string){
            this->t_error(ret_type_mismatch, p->m_attribute);
        }
    }

    // Create a symbol for the procedure and check there is none already
    // existing
    void check_call(Call *p)
    {
    }

    // For checking that this expressions type is boolean used in if/else and
    // while visits
    void check_pred(Expr* p)
    {
        if(p->m_attribute.m_basetype != bt_boolean)
            this->t_error(ifpred_err,p->m_attribute);
    }

    void check_assignment(Assignment* p)
    {
        if(p->m_lhs->m_attribute.m_basetype != p->m_expr.m_attribute.m_basetype)
            this->t_error(incompat_assign, p->m_attribute);
    }

    void check_string_assignment(StringAssignment* p)
    {
        if(p->m_lhs->m_attribute.m_basetype != bt_string)
            this->t_error(incompat_assign, p->m_attribute);
    }

    void check_array_access(ArrayAccess* p)
    {
        if(p->m_symname->m_attribute.m_basetype!=bt_string)
            this->t_error(no_array_var, this->attribute);

        if(p->m_expr->m_attribute.m_basetype!=bt_integer)
            this->t_error(array_index_error, this->attribute);
    }

    void check_array_element(ArrayElement* p)
    {
        if(p->m_symname->m_attribute.m_basetype!=bt_string)
            this->t_error(no_array_var, this->attribute);

        if(p->m_expr->m_attribute.m_basetype!=bt_integer)
            this->t_error(array_index_error, this->attribute);
    }

    // For checking boolean operations(and, or ...)
    void checkset_boolexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking arithmetic expressions(plus, times, ...)
    void checkset_arithexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // Called by plus and minus: in these cases we allow pointer arithmetics
    void checkset_arithexpr_or_pointer(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking relational(less than , greater than, ...)
    void checkset_relationalexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking equality ops(equal, not equal)
    void checkset_equalityexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking not
    void checkset_not(Expr* parent, Expr* child)
    {
    }

    // For checking unary minus
    void checkset_uminus(Expr* parent, Expr* child)
    {
    }

    void checkset_absolute_value(Expr* parent, Expr* child)
    {
    }

    void checkset_addressof(Expr* parent, Lhs* child)
    {
    }

    void checkset_deref_expr(Deref* parent,Expr* child)
    {
    }

    // Check that if the right-hand side is an lhs, such as in case of
    // addressof
    void checkset_deref_lhs(DerefVariable* p)
    {
        Basetype ptype = p->m_symname->m_attribute.m_basetype;
        if(ptype!=bt_intptr && ptype!=bt_charptr)
            this->t_error(expr_addressof_error, this->attribute);
    }

    void checkset_variable(Variable* p)
    {
        Basetype ptype = p->m_symname->m_attribute.m_basetype;
        if(ptype==bt_procedure)
            this->t_error(proc_as_var, this->attribute);
    }


  public:

    Typecheck(FILE* errorfile, SymTab* st) {
        m_errorfile = errorfile;
        m_st = st;
    }

    void visitProgramImpl(ProgramImpl* p)
    {
        check_for_one_main(p);
    }

    void visitProcImpl(ProcImpl* p)
    {   
        m_st->open_scope();
        add_proc_symbol(p);
        check_proc(p);
        m_st->close_scope();
    }

    void visitCall(Call* p)
    {
        check_call(p);
    }

    void visitNested_blockImpl(Nested_blockImpl* p)
    {

    }

    void visitProcedure_blockImpl(Procedure_blockImpl* p)
    {
    }

    void visitDeclImpl(DeclImpl* p)
    {
        add_decl_symbol(p);
    }

    void visitAssignment(Assignment* p)
    {
        check_assignment(p);
    }

    void visitStringAssignment(StringAssignment *p)
    {
        check_string_assignment(p);
    }

    void visitIdent(Ident* p)
    {
    }

    void visitReturn(Return* p)
    {
        check_return(p);
    }

    void visitIfNoElse(IfNoElse* p)
    {
        check_pred(p->m_expr);
    }

    void visitIfWithElse(IfWithElse* p)
    {
        check_pred(p->m_expr);
    }

    void visitWhileLoop(WhileLoop* p)
    {
        check_pred(p->m_expr);
    }

    void visitTInteger(TInteger* p)
    {
    }

    void visitTBoolean(TBoolean* p)
    {
    }

    void visitTCharacter(TCharacter* p)
    {
    }

    void visitTString(TString* p)
    {
    }

    void visitTCharPtr(TCharPtr* p)
    {
    }

    void visitTIntPtr(TIntPtr* p)
    {
    }

    void visitAnd(And* p)
    {
    }

    void visitDiv(Div* p)
    {
    }

    void visitCompare(Compare* p)
    {
    }

    void visitGt(Gt* p)
    {
    }

    void visitGteq(Gteq* p)
    {
    }

    void visitLt(Lt* p)
    {
    }

    void visitLteq(Lteq* p)
    {
    }

    void visitMinus(Minus* p)
    {
    }

    void visitNoteq(Noteq* p)
    {
    }

    void visitOr(Or* p)
    {
    }

    void visitPlus(Plus* p)
    {
    }

    void visitTimes(Times* p)
    {
    }

    void visitNot(Not* p)
    {
    }

    void visitUminus(Uminus* p)
    {
    }

    void visitArrayAccess(ArrayAccess* p)
    {
        check_array_access(p);
    }

    void visitIntLit(IntLit* p)
    {
        p->m_attribute.m_basetype = bt_integer;
    }

    void visitCharLit(CharLit* p)
    {
        p->m_attribute.m_basetype = bt_char;
    }

    void visitBoolLit(BoolLit* p)
    {
        p->m_attribute.m_basetype = bt_boolean;
    }

    void visitNullLit(NullLit* p)
    {
        p->m_attribute.m_basetype = bt_ptr;
    }

    void visitAbsoluteValue(AbsoluteValue* p)
    {
    }

    void visitAddressOf(AddressOf* p)
    {
    }

    void visitVariable(Variable* p)
    {
        checkset_variable(p);
    }

    void visitDeref(Deref* p)
    {
        checkset_deref_expr(p);
    }

    void visitDerefVariable(DerefVariable* p)
    {
        checkset_deref_lhs(p);
    }

    void visitArrayElement(ArrayElement* p)
    {
        check_array_element(p);
    }

    // Special cases
    void visitPrimitive(Primitive* p) {}
    void visitSymName(SymName* p) {}
    void visitStringPrimitive(StringPrimitive* p) {}
};


void dopass_typecheck(Program_ptr ast, SymTab* st)
{
    Typecheck* typecheck = new Typecheck(stderr, st);
    ast->accept(typecheck); // Walk the tree with the visitor above
    delete typecheck;
}
