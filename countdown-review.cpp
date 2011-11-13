#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <map>

using namespace std;

#define MAX_OPERATORS 4

typedef unsigned int uint32;
typedef uint32 (*operator_ptr_t)(uint32,uint32);

// Map container for standard math operators
// Global for performance
//map<operator_ptr_t, char> operators;

// Initialize map for operators
/*
void init_operators(){
    operators[add] = '+';
    operators[sub] = '-';
    operators[mult] = '*';
    operators[divide] = '/';
}
*/



// Converter int -> string
const string itos(uint32 i)
{
    stringstream s;
    s << i;
    return s.str();
}

// Functions-operators
inline uint32 add(uint32 left, uint32 right){
    return left + right;
}
inline uint32 sub(uint32 left, uint32 right){
    return left - right;
}
inline uint32 mult(uint32 left, uint32 right){
    return left * right;
}
inline uint32 divide(uint32 left, uint32 right){
    return left / right;
}


operator_ptr_t operators_list[] = {add, sub, mult, divide};
char operators_char_list[] = {'+','-','*','/'};


// Expression - has left and right branches and an operator between them.
class Expression {
public:
    //Expression(operator_ptr_t op_, Expression &lhs_, Expression &rhs_, list<uint32> numbers_list_):
    Expression(int op_, Expression &lhs_, Expression &rhs_, list<uint32> numbers_list_):
        op(op_),
        lhs(lhs_),
        rhs(rhs_),
        remaining_sources(numbers_list_)
    {
        value = operators_list[op](lhs.get_value(), rhs.get_value() );
    }
    Expression(uint32 value_, list<uint32> numbers_list_):
        op(0),
        lhs(*this),
        rhs(*this),
        remaining_sources(numbers_list_),
        value(value_)
    {}

    const uint32 get_value() {return value;}
    const list<uint32> &get_rem_sources() {return remaining_sources;}
    const string to_text() {
        if (&lhs == this) { return itos( value ); }
        else {
            return string("(" + \
                            lhs.to_text() + \
                            /*operators[op] + \ */
                            operators_char_list[op] + \
                            rhs.to_text() + \
                          ")");
        }
    }

private:
    //operator_ptr_t op;
    int op;
    Expression &lhs;
    Expression &rhs;
    list<uint32> remaining_sources;
    uint32 value;

};


// simple comparing of target to expression value
inline void compare(uint32 target, Expression *e){
    if (e->get_value() == target)
        cout << e->to_text() << " = " << target << endl;
}


/* Main function that generates math expressions recursively
 * target - target number
 * sources - list of numbers available
 * min_rem_sources - minimum number of numbers for the expression
 * counter - Converting generators to usual recursion needs internal
 *           flag to check recursion level. 
*/
const list<Expression *>
GenExpressions(uint32 target_, const list<uint32> &sources_,
                      uint32 min_rem_sources_, uint32 counter_)
{

    // Generates list of simple expressions from list of numbers
    vector<Expression *> simple_expr_list;
    int max_size = sources_.size();
    list<uint32>::const_iterator it = sources_.begin();
    for(int i=0; i<max_size; ++i, ++it){
        vector<uint32> temp(sources_.begin(), sources_.end());
        temp.erase(temp.begin()+i);
        Expression *res = new Expression((uint32)*it, list<uint32>(temp.begin(), temp.end()));
        simple_expr_list.insert(simple_expr_list.end(), res);
    }

    list<Expression *> expr_list;

    // If we are inside more than one call level then add simple
    // expressions to the full list
    if(counter_){
        // Extend list
        expr_list.insert(expr_list.end(), simple_expr_list.begin(), simple_expr_list.end() );
    // We are in the outer function call, we only nedd to compare our expressions to target
    }else{
        int max_size = simple_expr_list.size();
        for(int i=0; i<max_size; ++i){
            compare(target_, simple_expr_list[i]);
            delete simple_expr_list[i];
        }
    }

    if(sources_.size() >= (min_rem_sources_+2) ) {
        list<Expression *> lhs_list( GenExpressions(target_, sources_,
                                                    min_rem_sources_+1, counter_+1) );

        // Two loops for left and right branches of expression
        list<Expression *>::iterator lhs_it;
        for(lhs_it=lhs_list.begin(); lhs_it != lhs_list.end(); ++lhs_it) {
            list<Expression *> rhs_list( GenExpressions(target_, (*lhs_it)->get_rem_sources(),
                                                        min_rem_sources_, counter_+1) );
            uint32 left = (*lhs_it)->get_value();

            list<Expression *>::iterator rhs_it;
            for(rhs_it=rhs_list.begin(); rhs_it != rhs_list.end(); ++rhs_it){
                uint32 right = (*rhs_it)->get_value();

                // Optimization - avoid duplications like a+b,b+a or a*b,b*a.
                // We only calculate variant with biggest left part
                // Thus we also omit subtraction and division exceptions
                if ( left < right ) {
                    continue;
                }

                // Iterating through the list of math operators
                //map<operator_ptr_t, char>::iterator it;
                //for (it=operators.begin(); it != operators.end(); ++it){
                operator_ptr_t op;
                for (int it=0; it < MAX_OPERATORS; ++it){
                    op = operators_list[it];
                    
                    // Avoid 1/3, only 3/1
                    //if( (it->first == divide) && !( (left/right)*right == left ) ) {
                    //if( (it->first == divide) && (left % right) ) {
                    if( (op == divide) && (left % right) ) {
                        continue;
                    }

                    // Optimization - a*1 or b/1 are valid expressions but redundant
                    //if( ( right == 1 ) && (it->first == mult || it->first == divide) ) {
                    if( ( right == 1 ) && (op == mult || op == divide) ) {
                        continue;
                    }

                    // Avoid a-b=0
                    //if( ( it->first == sub ) && ( left == right ) ) {
                    if( ( op == sub ) && ( left == right ) ) {
                        continue;
                    }

                    // Create new 100% valid expression
                    //Expression *res = new Expression( it->first, **lhs_it, **rhs_it,
                    Expression *res = new Expression( it, **lhs_it, **rhs_it,
                                                      (*rhs_it)->get_rem_sources() );
                    
                    if(counter_){
                        expr_list.push_back(res);
                    }else{
                        compare(target_, res);
                        delete res;
                    }
                }
            }
        }
    }

    return expr_list;
}


int main(int argc, char **argv) {

    //init_operators();

    if(argc < 3) {
        cerr << "Usage: ./countdown <target> <num1> <num2>...<numN>" << endl;
        return 1;
    }
    
    uint32 target = atoi(argv[1]);

    list<uint32> input_numbers;
    for(int i=2; i<(argc); ++i) {
        input_numbers.push_back( atoi(argv[i]) );
    }

    GenExpressions(target, input_numbers, 0, 0);

    return 0;
}

