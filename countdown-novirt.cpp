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
    return left / right;
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


// Complex expression - has left and right branches and an operator between them
class ComplexExpression {
public:
    ComplexExpression(operator_ptr_t op_, ComplexExpression &lhs_, ComplexExpression &rhs_,
        vector<int> numbers_list_):
        op(op_),
        lhs(lhs_),
        rhs(rhs_),
        remaining_sources(numbers_list_)
    {
        value = op(lhs.get_value(), rhs.get_value() );
    }
    ComplexExpression(const int value_, vector<int> numbers_list_):
        op(NULL),
        lhs(*this),
        rhs(*this),
        remaining_sources(numbers_list_),
        value(value_)
    {}

    const int get_value() {return value;}
    const vector<int> get_rem_sources() {return remaining_sources;}
    const string to_text() {
        if (&lhs == this) { return itos( value ); }
        else {
            return string("(" + \
                            lhs.to_text() + \
                            operators[op] + \
                            rhs.to_text() + \
                          ")");
        }
    }

private:
    operator_ptr_t op;
    ComplexExpression &lhs;
    ComplexExpression &rhs;
    vector<int> remaining_sources;
    unsigned int value;

};


// simple comparing of target to expression value
inline void validate(int target, ComplexExpression *e){
    if (e->get_value() == target)
        cout << e->to_text() << " = " << target << endl;
}


/* Main function that generates math expressions
 * target - target number
 * sources - list of numbers available
 * operators - list of operators to use
 * min_rem_sources - minimum number of numbers for the expression
 * counter - Converting generators to usual recursion needs internal
 *           flag to check recursion level. 
*/
const vector<ComplexExpression *>
GenComplexExpressions(int target_, const vector<int> &sources_,
                      const int min_rem_sources_, const int counter_)
{

    // Generates list of simple expressions from list of numbers
    vector<ComplexExpression *> simple_expr_list;
    int max_size = sources_.size();
    for(int i=0; i<max_size; ++i){
        vector<int> temp(sources_);
        temp.erase(temp.begin()+i);
        ComplexExpression *res = new ComplexExpression(sources_[i], temp);
        simple_expr_list.insert(simple_expr_list.end(),res);
    }

    vector<ComplexExpression *> expr_list;

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
        vector<ComplexExpression *> lhs_list( GenComplexExpressions(target_, sources_,
                                                             min_rem_sources_+1,
                                                             counter_+1) );
        int max_i = lhs_list.size();
        for(int i=0; i<max_i; ++i) {
            vector<ComplexExpression *> rhs_list( GenComplexExpressions(target_, lhs_list[i]->get_rem_sources(),
                                                                        min_rem_sources_, counter_+1) );
            int left = lhs_list[i]->get_value();

            int max_j = rhs_list.size();
            for(int j=0; j<max_j; ++j){
                int right = rhs_list[j]->get_value();

                // Optimization - avoid duplications like a+b,b+a or a*b,b*a.
                // We only calculate variant with biggest left part
                // Thus we also omit subtraction and division exceptions
                if ( left < right ) {
                    continue;
                }

                map<operator_ptr_t, char>::iterator it;
                for (it=operators.begin(); it != operators.end(); ++it){
                    
                    // Avoid 1/3, only 3/1
                    if( (it->first == divide) && !( (left/right)*right == left ) ) {
                        continue;
                    }

                    // Optimization - a*1 or b/1 are valid expressions but redundant
                    if( ( right == 1 ) && (it->first == mult || it->first == divide) ) {
                        continue;
                    }

                    // Avoid a-b=0
                    if( ( it->first == sub ) && ( left == right ) ) {
                        continue;
                    }

                    ComplexExpression *res = new ComplexExpression( it->first,
                                                                    *lhs_list[i],
                                                                    *rhs_list[j],
                                                                    rhs_list[j]->get_rem_sources() );
                    
                    if(counter_){
                        expr_list.insert(expr_list.end(), res);
                    }else{
                        validate(target_, res);
                        delete res;
                    }
                }
            }
        }
    }

    cout << counter_ << "\t" << expr_list.size() << endl;
    return expr_list;
}


int main(int argc, char **argv) {

    init_operators();

    if(argc < 3) {
        cerr << "Usage: ./countdown <target> <num1> <num2>...<numN>" << endl;
        return 1;
    }
    
    int target = atoi(argv[1]);

    vector<int> input_numbers;
    for(int i=2; i<(argc); ++i) {
        input_numbers.push_back( atoi(argv[i]) );
    }

    GenComplexExpressions(target, input_numbers, 0, 0);

    return 0;
}

