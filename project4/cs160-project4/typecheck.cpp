#include <iostream>
#include <cstdio>
#include <cstring>

#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
using namespace std;


#include <typeinfo>

class Typecheck : public Visitor
{
  private:
    FILE* m_errorfile;
    SymTab* m_st;
    bool main_declared = false;

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
                size--;
            }
        }
        s->m_basetype = bt_procedure;
        s->m_return_type = p->m_type->m_attribute.m_basetype;

        //check if proc is main
        cout<<name<<endl;
        cout<<s->m_arg_type.size()<<endl;
        if(strcmp(name,"Main")==0){
            /*if(!main_declared)
                main_declared = true;
            else
                this->t_error(no_main, p->m_attribute);*/
            if(s->m_arg_type.size()!=0)
                this->t_error(nonvoid_main, p->m_attribute);
        }
        if(m_st->lookup(name)!=NULL)
            this->t_error(dup_proc_name, p->m_attribute);
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

    // only figure out the parameter type of Proc
    void process_Proc_symbol(ProcImpl* p){
        std::list<Decl_ptr>::iterator iter;
        char * name; Symbol *s = new Symbol();
        int size =0;
        for (iter = p->m_decl_list->begin(); iter != p->m_decl_list->end(); ++iter){
            static_cast<DeclImpl *>(*iter)->m_type->accept(this);
        }
        p->m_type->accept(this);
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
        //check if procedure exist and it is a procedure
        //chek number of argument matches
        //type of arguments matches
        //type of lhs and procedure's return type matches
        cout<<"Enter check call\n";
        std::list<Expr_ptr>::iterator iter;
        Symbol * s = m_st->lookup(p->m_symname->spelling());
        if(s==NULL)
            this->t_error(proc_undef, p->m_attribute);
        if(p->m_expr_list->size()!=s->m_arg_type.size())
            this->t_error(narg_mismatch, p->m_attribute);
        if(s->m_return_type!=p->m_lhs->m_attribute.m_basetype){
            cout<<s->m_basetype<<endl;
            cout<<p->m_lhs->m_attribute.m_basetype<<endl;
            this->t_error(call_type_mismatch, p->m_attribute);
        }

        std::vector<Basetype>::iterator iter2 = s->m_arg_type.begin();
        for(iter = p->m_expr_list->begin(); iter!=p->m_expr_list->end(); ++iter){
            if((*iter)->m_attribute.m_basetype!=*(iter2))
                this->t_error(arg_type_mismatch, p->m_attribute);
                ++iter2;
        }

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
            cout<<l<<endl<<r<<endl<<endl;
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
        p->m_attribute.m_basetype = bt_char;
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
        p->m_attribute.m_basetype = bt_char;
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
        Basetype c1 = child1->m_attribute.m_basetype, c2 = child2->m_attribute.m_basetype;
        if(c1 == bt_intptr || c1 == bt_charptr || c2 == bt_intptr || c2 == bt_charptr)
            this->t_error(expr_pointer_arithmetic_err, parent->m_attribute);
        if(c1!=bt_integer || c2!=bt_integer)
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_integer;
    }

    // Called by plus and minus: in these cases we allow pointer arithmetics
    void checkset_arithexpr_or_pointer(Expr* parent, Expr* child1, Expr* child2)
    {
        Basetype c1 = child1->m_attribute.m_basetype, c2 = child2->m_attribute.m_basetype;
        if(c2 == bt_charptr || c2 == bt_intptr)
            this->t_error(expr_pointer_arithmetic_err, parent->m_attribute);
        if(c1 == bt_charptr || c2 == bt_intptr){
            if(c2!=bt_integer)
                this->t_error(expr_pointer_arithmetic_err, parent->m_attribute);
        }
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
        if(c1 == c2 && (c1 == bt_boolean || c1 == bt_integer || c1 == bt_charptr || c1 == bt_intptr ||c1 == bt_char || c1 == bt_ptr)){

        }
        else if((c1 == bt_charptr || c1 == bt_intptr) && c2 == bt_ptr){

        }
        else if((c2 == bt_charptr || c2 == bt_intptr) && c1 == bt_ptr){

        }
        else
            this->t_error(expr_type_err, parent->m_attribute);
        parent->m_attribute.m_basetype = bt_boolean;
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
            this->t_error(expr_abs_error, parent->m_attribute);
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
        if(s->m_basetype!=bt_charptr)
            p->m_attribute.m_basetype = bt_char;
        else if (s->m_basetype!=bt_intptr)
            p->m_attribute.m_basetype = bt_integer;
        else
            this->t_error(invalid_deref, p->m_attribute);
    }

    void checkset_variable(Variable* p)
    {
        Symbol * s = m_st->lookup(p->m_symname->spelling());
        if(s == NULL)
            this->t_error(var_undef, p->m_attribute);
        p->m_attribute.m_basetype = s->m_basetype;
    }

    void checkset_ident(Ident* p){
        Symbol * s = m_st->lookup(p->m_symname->spelling());
        if(s == NULL)
            this->t_error(var_undef, p->m_attribute);
        p->m_attribute.m_basetype = s->m_basetype;
    }


  public:
    void default_rule(Visitable* p){
        static_cast<Program*>(p)->m_attribute.m_scope = m_st->get_scope();
        p->visit_children(this);
    }

    Typecheck(FILE* errorfile, SymTab* st) {
        m_errorfile = errorfile;
        m_st = st;
    }

    void visitProgramImpl(ProgramImpl* p)
    {
        cout<<p->m_attribute.lineno<<endl;
        default_rule(p);
        check_for_one_main(p);
    }

    void visitProcImpl(ProcImpl* p)
    {
        process_Proc_symbol(p);
        add_proc_symbol(p);
        m_st->open_scope();
        default_rule(p);
        m_st->close_scope();
        check_proc(p);
    }

    void visitCall(Call* p)
    {
        default_rule(p);
        check_call(p);
    }

    void visitNested_blockImpl(Nested_blockImpl* p)
    {
        default_rule(p);
    }

    void visitProcedure_blockImpl(Procedure_blockImpl* p)
    {
        default_rule(p);
    }

    void visitDeclImpl(DeclImpl* p)
    {
        default_rule(p);
        add_decl_symbol(p);
    }

    void visitAssignment(Assignment* p)
    {
        default_rule(p);
        check_assignment(p);
    }

    void visitStringAssignment(StringAssignment *p)
    {
        default_rule(p);
        check_string_assignment(p);
    }

    void visitIdent(Ident* p)
    {
        default_rule(p);
        checkset_ident(p);
    }

    void visitReturn(Return* p)
    {
        default_rule(p);
        check_return(p);
    }

    void visitIfNoElse(IfNoElse* p)
    {
        default_rule(p);
        check_pred(p->m_expr,1);
    }

    void visitIfWithElse(IfWithElse* p)
    {
        default_rule(p);
        check_pred(p->m_expr,1);
    }

    void visitWhileLoop(WhileLoop* p)
    {
        default_rule(p);
        check_pred(p->m_expr,2);
    }

    void visitTInteger(TInteger* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_integer;
    }

    void visitTBoolean(TBoolean* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_boolean;
    }

    void visitTCharacter(TCharacter* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_char;
    }

    void visitTString(TString* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_string;
    }

    void visitTCharPtr(TCharPtr* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_charptr;
    }

    void visitTIntPtr(TIntPtr* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_intptr;
    }

    void visitAnd(And* p)
    {
        default_rule(p);
        checkset_boolexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitDiv(Div* p)
    {
        default_rule(p);
        checkset_arithexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitCompare(Compare* p)
    {
        default_rule(p);
        checkset_equalityexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitGt(Gt* p)
    {
        default_rule(p);
        checkset_relationalexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitGteq(Gteq* p)
    {
        default_rule(p);
        checkset_relationalexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitLt(Lt* p)
    {
        default_rule(p);
        checkset_relationalexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitLteq(Lteq* p)
    {
        default_rule(p);
        checkset_relationalexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitMinus(Minus* p)
    {
        default_rule(p);
        checkset_arithexpr_or_pointer(p,p->m_expr_1,p->m_expr_2);
    }

    void visitNoteq(Noteq* p)
    {
        default_rule(p);
        checkset_equalityexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitOr(Or* p)
    {
        default_rule(p);
        checkset_boolexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitPlus(Plus* p)
    {
        default_rule(p);
        checkset_arithexpr_or_pointer(p,p->m_expr_1,p->m_expr_2);
    }

    void visitTimes(Times* p)
    {
        default_rule(p);
        checkset_arithexpr(p,p->m_expr_1,p->m_expr_2);
    }

    void visitNot(Not* p)
    {
        default_rule(p);
        checkset_not(p,p->m_expr);
    }

    void visitUminus(Uminus* p)
    {
        default_rule(p);
        checkset_uminus(p,p->m_expr);
    }

    void visitArrayAccess(ArrayAccess* p)
    {
        default_rule(p);
        check_array_access(p);
    }

    void visitIntLit(IntLit* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_integer;
    }

    void visitCharLit(CharLit* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_char;
    }

    void visitBoolLit(BoolLit* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_boolean;
    }

    void visitNullLit(NullLit* p)
    {
        default_rule(p);
        p->m_attribute.m_basetype = bt_ptr;
    }

    void visitAbsoluteValue(AbsoluteValue* p)
    {
        cout<<"Enter here\n";
        default_rule(p);
        checkset_absolute_value(p,p->m_expr);
    }

    void visitAddressOf(AddressOf* p)
    {
        default_rule(p);
        checkset_addressof(p,p->m_lhs);
    }

    void visitVariable(Variable* p)
    {
        default_rule(p);
        checkset_variable(p);
    }

    void visitDeref(Deref* p)
    {
        default_rule(p);
        checkset_deref_expr(p,p->m_expr);
    }

    void visitDerefVariable(DerefVariable* p)
    {
        default_rule(p);
        checkset_deref_lhs(p);
    }

    void visitArrayElement(ArrayElement* p)
    {
        default_rule(p);
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
