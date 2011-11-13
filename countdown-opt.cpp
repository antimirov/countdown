#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;


// Converter int -> string
const string itos(int i)
{
    stringstream s;
    s << i;
    return s.str();
}

// Functions-operators
inline int add(int left, int right){
    return left + right;
}
inline int sub(int left, int right){
    return left - right;
}
inline int mult(int left, int right){
    return left * right;
}
inline int divide(int left, int right){
    if( (left/right)*right == left ){
        return left / right;
    }
    return 0;
}


// Map container for standard math operators
typedef int (*operator_ptr_t)(int,int);
map<operator_ptr_t, char> operators;

// Initialize map for operators
void init_operators(){
    operators[add] = '+';
    operators[sub] = '-';
    operators[mult] = '*';
    operators[divide] = '/';
}

// Expression classes
class Expression {
public:
    virtual ~Expression(){};
    virtual const string to_text() = 0;
    virtual const int get_value() = 0;
    virtual const vector<int> get_rem_sources() = 0;
    virtual void cleanup() = 0;
};


// Simple expression - stores its value and set of numbers left
class SimpleExpression: public Expression {
public:
    explicit SimpleExpression(const int value_, vector<int> numbers_list_):
        value(value_),
        remaining_sources(numbers_list_)
    {++counter;}
    
    const int get_value() {return value;}
    const vector<int> get_rem_sources() {return remaining_sources;}
    const string to_text(){ return itos( value ); }
    void cleanup(){}

    static int counter;

private:
    unsigned int value;
    vector<int> remaining_sources;
};


// Complex expression - has left and right branches and an operator between them
class ComplexExpression: public Expression {
public:
    explicit ComplexExpression(operator_ptr_t op_, Expression &lhs_, Expression &rhs_,
        vector<int> numbers_list_):
        op(op_),
        lhs(lhs_),
        rhs(rhs_),
        remaining_sources(numbers_list_)
    {
        value = op(lhs.get_value(), rhs.get_value() );
        ++counter;
    }
    ~ComplexExpression(){--counter;}

    const int get_value() {return value;}
    const vector<int> get_rem_sources() {return remaining_sources;}
    const string to_text() {
        return string("(" + \
                        lhs.to_text() + \
                        operators[op] + \
                        rhs.to_text() + \
                      ")");
    }
    void cleanup(){
        delete &rhs;
        //delete lhs;
    }

    static int counter;

private:
    operator_ptr_t op;
    Expression &lhs;
    Expression &rhs;
    vector<int> remaining_sources;
    unsigned int value;

};
// END: Expression classes

int SimpleExpression::counter;
int ComplexExpression::counter;

// simple comparing target to expression value
inline void validate(int target, Expression *e){
    if (e->get_value() == target)
        cout << e->to_text() << " = " << target << endl;
}


// Generates list of simple expressions from list of numbers
const vector<Expression *>
GenSimpleExpressions(const vector<int> &sources_){

    vector<Expression *> expr_list;
    for(int i=0; i<sources_.size(); ++i){
        vector<int> temp(sources_);
        temp.erase(temp.begin()+i);
        SimpleExpression *res = new SimpleExpression(sources_[i], temp);
        expr_list.push_back(res);
    }
    
    return expr_list;
}


/* Main function that generates math expressions
 * target - target number
 * sources - list of numbers available
 * operators - list of operators to use
 * min_rem_sources - minimum number of numbers for the expression
 * counter - Converting generators to usual recursion needs internal
 *           flag to check recursion level. 
*/
const vector<Expression *>
GenComplexExpressions(int target_, const vector<int> &sources_,
                      map<operator_ptr_t, char> &operators_,
                      const int min_rem_sources_, const int counter_)
{
    vector<Expression *> expr_list;

    vector<Expression *> simple_expr_list( GenSimpleExpressions(sources_) );
    if(counter_){
        // Extend list
        expr_list.insert(expr_list.end(), simple_expr_list.begin(), simple_expr_list.end() );
    }else{
        for(int i=0; i<simple_expr_list.size(); ++i){
            validate(target_, simple_expr_list[i]);
            delete simple_expr_list[i];
        }
    }

    if(sources_.size() >= (min_rem_sources_+2) ) {
        vector<Expression *> lhs_list( GenComplexExpressions(target_, sources_, operators_,
                                                             min_rem_sources_+1,
                                                             counter_+1) );
        for(int i=0; i<lhs_list.size(); ++i) {
            vector<Expression *> rhs_list( GenComplexExpressions(target_, lhs_list[i]->get_rem_sources(),
                                                                 operators_, min_rem_sources_, counter_+1) );
            for(int j=0; j<rhs_list.size(); ++j){
                map<operator_ptr_t, char>::iterator it;
                // Optimization - avoid duplications like a+b,b+a or a*b,b*a.
                // We only calculate variant with biggest left part
                // Thus we also omit subtraction and division exceptions
                if ( lhs_list[i]->get_value() < rhs_list[j]->get_value() ) {
                    continue;
                }
                for (it=operators.begin(); it != operators.end(); ++it){
                    
                    // Optimization - a*1 or b/1 are valid expressions but redundant
                    //if( ( rhs_list[j]->get_value() == 1 ) && (it->first == mult && it->first == divide) ){
                    //    continue;
                    //}

                    ComplexExpression *res = new ComplexExpression( it->first,
                                                                    *lhs_list[i],
                                                                    *rhs_list[j],
                                                                    rhs_list[j]->get_rem_sources() );
                    if( res->get_value() == 0 ){
                        delete res;
                        continue;
                    }
                    
                    if(counter_){
                        expr_list.insert(expr_list.end(), res);
                    }else{
                        validate(target_, res);
                        delete res;
                        res = NULL;
                    }
                }
            //delete rhs_list
            if(j){
                //rhs_list[j-1]->cleanup();
            }
            }
        }
    }

    return expr_list;
}


int main(int argc, char **argv) {

    init_operators();

    if(argc < 3) {
        cout << "Usage: ./countdown <target> <num1> <num2>...<numN>" << endl;
        exit(1);
    }
    
    int target = atoi(argv[1]);

    vector<int> input_numbers;
    for(int i=2; i<(argc); ++i) {
        input_numbers.push_back( atoi(argv[i]) );
    }

    GenComplexExpressions(target, input_numbers, operators, 0, 0);

    cerr << "SimpleExpression::counter: " << SimpleExpression::counter << endl;
    cerr << "ComplexExpression::counter: " << ComplexExpression::counter << endl;

    return 0;
}

