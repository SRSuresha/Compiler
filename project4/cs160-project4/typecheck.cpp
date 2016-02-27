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
                exit(2);
            case nonvoid_main:
                fprintf(m_errorfile, "error: the Main procedure has arguments\n");
                exit(3);
            case dup_proc_name:
                fprintf(m_errorfile, "error: duplicate procedure names in same scope\n");
                exit(4);
            case dup_var_name:
                fprintf(m_errorfile, "error: duplicate variable names in same scope\n");
                exit(5);
            case proc_undef:
                fprintf(m_errorfile, "error: call to undefined procedure\n");
                exit(6);
            case var_undef:
                fprintf(m_errorfile, "error: undefined variable\n");
                exit(7);
            case narg_mismatch:
                fprintf(m_errorfile, "error: procedure call has different number of args than declartion\n");
                exit(8);
            case arg_type_mismatch:
                fprintf(m_errorfile, "error: argument type mismatch\n");
                exit(9);
            case ret_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in return statement\n");
                exit(10);
            case call_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in procedure call args\n");
                exit(11);
            case ifpred_err:
                fprintf(m_errorfile, "error: predicate of if statement is not boolean\n");
                exit(12);
            case whilepred_err:
                fprintf(m_errorfile, "error: predicate of while statement is not boolean\n");
                exit(13);
            case array_index_error:
                fprintf(m_errorfile, "error: array index not integer\n");
                exit(14);
            case no_array_var:
                fprintf(m_errorfile, "error: attempt to index non-array variable\n");
                exit(15);
            case incompat_assign:
                fprintf(m_errorfile, "error: type of expr and var do not match in assignment\n");
                exit(16);
            case expr_type_err:
                fprintf(m_errorfile, "error: incompatible types used in expression\n");
                exit(17);
            case expr_abs_error:
                fprintf(m_errorfile, "error: absolute value can only be applied to integers and strings\n");
                exit(17);
            case expr_pointer_arithmetic_err:
                fprintf(m_errorfile, "error: invalid pointer arithmetic\n");
                exit(18);
            case expr_addressof_error:
                fprintf(m_errorfile, "error: AddressOf can only be applied to integers, chars, and indexed strings\n");
                exit(19);
            case invalid_deref:
                fprintf(m_errorfile, "error: Deref can only be applied to integer pointers and char pointers\n");
                exit(20);
            default:
                fprintf(m_errorfile, "error: no good reason\n");
                exit(21);
        }
    }


    // Type Checking
    // WRITEME: You need to implement type-checking for this project

    // Check that there is one and only one main
    void check_for_one_main(ProgramImpl* p)
    {
        Symbol * s = m_st->lookup("Main");
        if(s==NULL || s->m_basetype!=bt_procedure)
            this->t_error(no_main,p->m_attribute);
    }

    // Create a symbol for the procedure and check there is none already
    // existing
    void add_proc_symbol(ProcImpl* p)
    {
        std::list<Decl_ptr>::iterator iter;
        std::list<SymName_ptr>::iterator iter2;
        char * name; Symbol *s = new Symbol();
        int size =0;
        name =strdup(p->m_symname->spelling());
        for (iter = p->m_decl_list->begin(); iter != p->m_decl_list->end(); ++iter){
            size = static_cast<DeclImpl *>(*iter)->m_symname_list->size();
            while(size>0){
                s->m_arg_type.push_back(static_cast<DeclImpl *>(*iter)->m_type->m_attribute.m_basetype);
            }
        }
        s->m_basetype = bt_procedure;
        s->m_return_type = p->m_type->m_attribute.m_basetype;
        if(!m_st->insert(name,s))
            this->t_error(dup_proc_name, p->m_attribute);
    }

    // Add symbol table information for all the declarations following
    void add_decl_symbol(DeclImpl* p)
    {
        std::list<SymName_ptr>::iterator iter;
        char * name; Symbol *s;
        for(iter = p->m_symname_list->begin(); iter !=p->m_symname_list->end(); ++iter){
            name = strdup((*iter)->spelling());
            s = new Symbol();
            s->m_basetype = p->m_type->m_attribute.m_basetype;

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
        real_type =  static_cast<Return *>(static_cast<Procedure_blockImpl *>(p->m_procedure_block)->m_return_stat)->m_expr->m_attribute.m_basetype;
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
    //index 1 means it is a ifelse, index 2 means it is a while
    void check_pred(Expr* p, int index)
    {
        if(p->m_attribute.m_basetype!=bt_boolean){
            if(index == 1)
                this->t_error(ifpred_err, p->m_attribute);
            else
                this->t_error(whilepred_err, p->m_attribute);
        }
    }

    //if lhs is charptr or intptr, the rhs can be bt_ptr
    //otherwise type should be the same
    void check_assignment(Assignment* p)
    {
        Basetype l = p->m_lhs->m_attribute.m_basetype, r = p->m_expr->m_attribute.m_basetype;
        if(l!=r){
            if((l==bt_intptr || l == bt_charptr) && r == bt_ptr){}
            else
                this->t_error(incompat_assign, p->m_attribute);
        }
    }

    //only need to check if lhs's type is String
    void check_string_assignment(StringAssignment* p)
    {
        if(p->m_lhs->m_attribute.m_basetype!=bt_string)
            this->t_error(incompat_assign, p->m_attribute);
    }


    void check_array_access(ArrayAccess* p)
    {
        Symbol * s = m_st->lookup(p->m_symname->spelling());
        if(s == NULL)
            this->t_error(var_undef, p->m_attribute);
        if(s->m_basetype!=bt_string)
            this->t_error(no_array_var, p->m_attribute);

        if(p->m_expr->m_attribute.m_basetype!=bt_integer)
            this->t_error(array_index_error, p->m_attribute);
    }

    void check_array_element(ArrayElement* p)
    {
        Symbol * s = m_st->lookup(p->m_symname->spelling());
        if(s == NULL)
            this->t_error(var_undef, p->m_attribute);
        if(s->m_basetype!=bt_string)
            this->t_error(no_array_var, p->m_attribute);

        if(p->m_expr->m_attribute.m_basetype!=bt_integer)
            this->t_error(array_index_error, p->m_attribute);
    }

    // For checking boolean operations(and, or ...)
    void checkset_boolexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        if(child1->m_attribute.m_basetype!=bt_boolean || child2->m_attribute.m_basetype!=bt_boolean)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_boolean;
    }

    // For checking arithmetic expressions(plus, times, ...)
    void checkset_arithexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        if(child1->m_attribute.m_basetype!=bt_integer || child2->m_attribute.m_basetype!=bt_integer)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_integer;
    }

    // Called by plus and minus: in these cases we allow pointer arithmetics
    void checkset_arithexpr_or_pointer(Expr* parent, Expr* child1, Expr* child2)
    {
        Basetype c1 = child1->m_attribute.m_basetype, c2 = child2->m_attribute.m_basetype;
        if((c1!=bt_integer && c1!=bt_charptr && c1!=bt_intptr) || c2!=bt_integer)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = c1;
    }

    // For checking relational(less than , greater than, ...)
    // assuming we can only use >, < in integer
    void checkset_relationalexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        if(child1->m_attribute.m_basetype!=bt_integer || child2->m_attribute.m_basetype!=bt_integer)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_boolean;
    }

    // For checking equality ops(equal, not equal)
    void checkset_equalityexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        Basetype c1 = child1->m_attribute.m_basetype, c2 = child2->m_attribute.m_basetype;

    }

    // For checking not
    void checkset_not(Expr* parent, Expr* child)
    {
        if(child->m_attribute.m_basetype != bt_boolean)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_boolean;
    }

    // For checking unary minus
    void checkset_uminus(Expr* parent, Expr* child)
    {
        if(child->m_attribute.m_basetype != bt_integer)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_integer;
    }

    void checkset_absolute_value(Expr* parent, Expr* child)
    {
        Basetype c = child->m_attribute.m_basetype;
        if(c != bt_string && c != bt_integer)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_integer;
    }

    void checkset_addressof(Expr* parent, Lhs* child)
    {
        Basetype c = child->m_attribute.m_basetype;
        if(c ==bt_char)
            parent->m_attribute.m_basetype = bt_charptr;
        else if (c ==bt_integer)
            parent->m_attribute.m_basetype = bt_intptr;
        else
            this->t_error(expr_addressof_error, parent->m_attribute);
    }

    void checkset_deref_expr(Deref* parent,Expr* child)
    {
        Basetype c = child->m_attribute.m_basetype;
        if(c ==bt_charptr)
            parent->m_attribute.m_basetype = bt_char;
        else if (c ==bt_intptr)
            parent->m_attribute.m_basetype = bt_integer;
        else
            this->t_error(invalid_deref, parent->m_attribute);
    }

    // Check that if the right-hand side is an lhs, such as in case of
    // addressof
    void checkset_deref_lhs(DerefVariable* p)
    {
        Symbol * s = m_st->lookup(p->m_symname->spelling());
        if(s == NULL)
            this->t_error(var_undef, p->m_attribute);
        if(s->m_basetype!=bt_charptr && s->m_basetype!=bt_intptr)
            this->t_error(invalid_deref, p->m_attribute);
    }

    void checkset_variable(Variable* p)
    {
        Symbol * s = m_st->lookup(p->m_symname->spelling());
        if(s == NULL)
            this->t_error(var_undef, p->m_attribute);
    }


  public:

    Typecheck(FILE* errorfile, SymTab* st) {
        m_errorfile = errorfile;
        m_st = st;
    }

    void visitProgramImpl(ProgramImpl* p)
    {
    }

    void visitProcImpl(ProcImpl* p)
    {
    }

    void visitCall(Call* p)
    {
        //check if procedure exist and it is a procedure
        //chek number of argument matches 
        //type of arguments matches
        //type of lhs and procedure's return type matches
    }

    void visitNested_blockImpl(Nested_blockImpl* p)
    {
    }

    void visitProcedure_blockImpl(Procedure_blockImpl* p)
    {
    }

    void visitDeclImpl(DeclImpl* p)
    {
    }

    void visitAssignment(Assignment* p)
    {
    }

    void visitStringAssignment(StringAssignment *p)
    {
    }

    void visitIdent(Ident* p)
    {
    }

    void visitReturn(Return* p)
    {
    }

    void visitIfNoElse(IfNoElse* p)
    {
    }

    void visitIfWithElse(IfWithElse* p)
    {
    }

    void visitWhileLoop(WhileLoop* p)
    {
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
    }

    void visitIntLit(IntLit* p)
    {
    }

    void visitCharLit(CharLit* p)
    {
    }

    void visitBoolLit(BoolLit* p)
    {
    }

    void visitNullLit(NullLit* p)
    {
    }

    void visitAbsoluteValue(AbsoluteValue* p)
    {
    }

    void visitAddressOf(AddressOf* p)
    {
    }

    void visitVariable(Variable* p)
    {
    }

    void visitDeref(Deref* p)
    {
    }

    void visitDerefVariable(DerefVariable* p)
    {
    }

    void visitArrayElement(ArrayElement* p)
    {
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
