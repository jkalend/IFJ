#include "parser.h"
#include "scanner.h"
#include "stack.h"
#include "htab.h"
#include "error.h"
#include "math.h"

// TODO
// TODO
//
// we are unable to accept inbuilt functions as of now
//
// FIXME
// FIXME

Token *tmp_token = NULL;
htab_t *glob_tab = NULL;
htab_t *temporary_tab = NULL;
TStack *local_tabs = NULL;
htab_pair_t *in_func = NULL;
htab_pair_t *in_assign = NULL;
bool in_param_def = false;
int tmp_counter = 0;
unsigned int relation_operator = 0;

const unsigned int PREC_TABLE[14][14] = { //TODO
        {P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_OPEN, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_CLOSE},
        {P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_OPEN, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_CLOSE},
        {P_OPEN,P_OPEN,P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_OPEN, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_CLOSE},
        {P_OPEN,P_OPEN,P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_OPEN, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_CLOSE},
        {P_OPEN,P_OPEN,P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_OPEN, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_CLOSE},
        {P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, [7] = P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, [13] = P_CLOSE},
        {P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_EQUAL, P_OPEN, P_OPEN, P_OPEN, [12] = P_OPEN, P_EQUAL},
        {P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, [7] = P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, P_CLOSE, [13] = P_CLOSE},
        {P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_CLOSE, [9] = P_CLOSE, P_CLOSE, P_CLOSE, P_OPEN, P_CLOSE},
        {P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_CLOSE, P_OPEN, [11] = P_CLOSE, P_OPEN, P_CLOSE},
        {P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_CLOSE, P_OPEN, [11] = P_CLOSE, P_OPEN, P_CLOSE},
        {P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN, [8] = P_OPEN, [12] = P_OPEN},
        {[6] = P_EQUAL},
        {P_OPEN, P_OPEN, P_OPEN, P_OPEN, P_OPEN,P_OPEN, P_OPEN, P_EQUAL, P_OPEN, [12] = P_OPEN, P_EQUAL}
};


TData *stack_data(int value, int type) { //unsigned?
    TData *ptr = malloc(sizeof(TData));
    if (ptr != NULL) {
        ptr->value = value;
        ptr->type = type;
        ptr->htab = NULL;
        ptr->bucket = NULL;
    }
    return ptr;
}

void get_next_token(Token **token, bool *keep_token, bool *return_back) {
    if (*keep_token && *return_back) {
        Token *tmp = *token;
        *token = tmp_token;
        tmp_token = tmp;
    }
    else if (!(*keep_token) && *return_back) {
        Token *tmp = *token;
        *token = tmp_token;
        tmp_token = tmp;
        *return_back = false;
    }
    else if (!(*keep_token)){
        /* copy previous token to temporary token */
        tmp_token->line = (*token)->line;
        tmp_token->value = (*token)->value;
        tmp_token->type = (*token)->type;

        /* get new token from scanner */
        get_token(*token);
    }
    *keep_token = false;
}

int apply_rule(TStack *stack, unsigned int val) {
    switch (val) {
        case 0: exit(1); /* syntax error */
        case 1:
            stack_push(stack, stack_data(N_ST_LIST, T_NONTERM));
            stack_push(stack, stack_data(T_VALID, T_TERM));
            break;
        case 2:
            stack_push(stack, stack_data(N_ST_LIST, T_NONTERM));
            stack_push(stack, stack_data(N_ST, T_NONTERM));
            break;
        case 3:
            stack_push(stack, stack_data(T_EOF, T_TERM));
            break;
        case 4:
            stack_push(stack, stack_data(T_END, T_TERM));
            break;
        case 5:
            stack_push(stack, stack_data(T_RIGHT_BRACE, T_TERM));
            stack_push(stack, stack_data(N_BODY, T_NONTERM));
            stack_push(stack, stack_data(T_LEFT_BRACE, T_TERM));
            stack_push(stack, stack_data(T_TYPE, T_TERM));
            stack_push(stack, stack_data(T_DOUBLE_DOT, T_TERM));
            stack_push(stack, stack_data(T_RIGHT_BRACKET, T_TERM));
            stack_push(stack, stack_data(N_PARAM_DEF, T_NONTERM));
            stack_push(stack, stack_data(T_LEFT_BRACKET, T_TERM));
            stack_push(stack, stack_data(T_IDENTIFIER, T_TERM));
            stack_push(stack, stack_data(KW_FUNCTION, T_KW));
            break;
        case 6:
            stack_push(stack, stack_data(N_SMALL_ST, T_NONTERM));
            stack_push(stack, stack_data(T_VAR, T_TERM));
            stack_push(stack, stack_data(KW_GLOBAL, T_KW));
            break;
        case 7:
            stack_push(stack, stack_data(T_SEMICOLON, T_TERM));
            stack_push(stack, stack_data(N_EXPR, T_NONTERM));
            break;
        case 8:
           // stack_push(stack, stack_data(T_SEMICOLON, T_TERM));
            //stack_push(stack, stack_data(N_EXPR, T_NONTERM));
            //stack_push(stack, stack_data(T_IDENTIFIER, T_TERM));
            break;
        case 9:
            stack_push(stack, stack_data(N_SMALL_ST, T_NONTERM));
            stack_push(stack, stack_data(T_VAR, T_TERM));
            break;
        case 10:
            stack_push(stack, stack_data(T_SEMICOLON, T_TERM));
            stack_push(stack, stack_data(N_EXPR, T_NONTERM));
            stack_push(stack, stack_data(KW_RETURN, T_KW));
            break;
        case 11:
            stack_push(stack, stack_data(T_RIGHT_BRACE, T_TERM));
            stack_push(stack, stack_data(N_BODY, T_NONTERM));
            stack_push(stack, stack_data(T_LEFT_BRACE, T_TERM));
            stack_push(stack, stack_data(KW_ELSE, T_KW));
            stack_push(stack, stack_data(T_RIGHT_BRACE, T_TERM));
            stack_push(stack, stack_data(N_BODY, T_NONTERM));
            stack_push(stack, stack_data(T_LEFT_BRACE, T_TERM));
            stack_push(stack, stack_data(N_EXPR, T_NONTERM));
            stack_push(stack, stack_data(KW_IF, T_KW));
            break;
        case 12:
            stack_push(stack, stack_data(T_RIGHT_BRACE, T_TERM));
            stack_push(stack, stack_data(N_BODY, T_NONTERM));
            stack_push(stack, stack_data(T_LEFT_BRACE, T_TERM));
            stack_push(stack, stack_data(N_EXPR, T_NONTERM));
            stack_push(stack, stack_data(KW_WHILE, T_KW));
            break;
        case 13:
            stack_push(stack, stack_data(T_SEMICOLON, T_TERM));
            break;
        case 14:
            stack_push(stack, stack_data(T_SEMICOLON, T_TERM));
            stack_push(stack, stack_data(N_EXPR, T_NONTERM));
            stack_push(stack, stack_data(T_ASSIGN, T_TERM));
            break;
        case 15:
            stack_push(stack, stack_data(T_SEMICOLON, T_TERM));
            stack_push(stack, stack_data(N_EXPR, T_NONTERM));
            break;
        case 16:
            stack_push(stack, stack_data(N_PARAM_LIST, T_NONTERM));
            stack_push(stack, stack_data(N_PARAM_NAME, T_NONTERM));
            stack_push(stack, stack_data(T_TYPE, T_TERM));
            break;
        case 17:
            stack_push(stack, stack_data(T_VAR, T_TERM));
            break;
        case 18:
            /* eps */
            break;
        case 19:
            stack_push(stack, stack_data(N_PARAM_DEF, T_NONTERM));
            stack_push(stack, stack_data(T_COMMA, T_TERM));
            break;
        case 20:
            stack_push(stack, stack_data(N_BODY, T_NONTERM));
            stack_push(stack, stack_data(N_ST, T_NONTERM));
            break;
        
        case 21:
        case 22:
            /* eps */
            break;
        default: fprintf(stderr, "no rule found\n");
                 exit(1);
    }
    return 1;
}

void prec_index(const Token *token, unsigned int *rc, int symbol) {
    if (symbol > -1 && symbol < 15) {
        *rc = symbol;
        return;
    }
    switch (token->type) {

        case T_IDENTIFIER: //func
            *rc = 12;
            break;
        case T_STRING: case T_INT: case T_FLOAT: case T_VAR:
            *rc = 5;
            break;
        case T_COMMA:
            *rc = 13;
            break;
        case T_EOF: case T_END:
            //FIXME
            break;
        case T_PLUS:
            *rc = 2;
            break;
        case T_MINUS:
            *rc = 3;
            break;
        case T_CONCATENATE:
            *rc = 4;
            break;
        case T_MULTIPLY:
            *rc = 0;
            break;
        case T_DIVIDE:
            *rc = 1;
            break;
        case T_LESS: case T_LESS_EQUAL: case T_GREATER: case T_GREATER_EQUAL:
            *rc = 8;
            break;
        case T_EQUAL:
            *rc = 9;
            break;
        case T_NOT_EQUAL:
            *rc = 10;
            break;
        case T_LEFT_BRACKET:
            *rc = 6;
            break;
        case T_RIGHT_BRACKET:
            *rc = 7;
            break;
        case T_ERROR:
            //FIXME
            break;
        case T_SEMICOLON: case T_LEFT_BRACE:
            //somehow return the token to the LL stack
            break;
        default:
            exit(1);
    }
}

int reduce(TStack *stack, TStack *shelf, TStack *temps) {

    if (stack_top(stack)->value == P_E) {
        stack_push(shelf, stack_pop(stack));
        if (stack_top(stack)->value == P_END) return -1; //empty expr
    }
    unsigned int res = 0;
    bool fn = false;
    const htab_pair_t *last_fn;

    while (stack_top(stack)->value != P_OPEN && stack_top(stack)->value != P_END) {
        const TData *data = stack_pop(stack);
        stack_push(shelf, data);
        res += data->value;
        if (data->value == P_FN) {fn = true; last_fn = data->bucket;}
    }
    if (stack_top(stack)->value == P_OPEN) {
        const TData *data = stack_pop(stack);
        res += data->value;
        stack_push(shelf, data);
    } else if (stack_top(stack)->value == P_END) {
        exit(BAD_SYNTAX);
    }

    //could it be simpler? yes, but 3AC gotta go somewhere
    int cnt = 0;
    DataType op_one;
    DataType op_two;
    char number[100];

    long long number_size = (long long)((ceil(log10(tmp_counter))+1)*sizeof(char));
    snprintf(number, number_size, "%d", tmp_counter);
    char tmp[100] = TEMP_VAR_PREFIX;
    strcat(tmp, number);

    TData *data;
    htab_pair_t *pair;
    if (res > 34 && res != 59) {
        tmp_counter++;
        pair = htab_insert(temporary_tab, NULL, tmp);
        data = stack_data(P_I, P_I);
        data->bucket = pair;
    }

    switch (res) {
        case 34:
            while (cnt < 3) {stack_pop(shelf); cnt++;}
            if (cnt != 3) goto cleanup;
            stack_push(stack, stack_data(P_E, P_E));
            printf("1p ");
            return 1;
        case 63: // multiplication
            while (cnt < 5) {if(stack_pop(shelf) != NULL) {break;} cnt++;}
            if (cnt != 5) goto cleanup;
            op_one = stack_pop(temps)->type;
            op_two = stack_pop(temps)->type;
            if (op_one == D_FLOAT || op_two == D_FLOAT) {
                pair->value_type = D_FLOAT;
            } else if (op_one == D_INT && op_two == D_INT) {
                pair->value_type = D_INT;
            }

            printf("2p ");
            stack_push(temps, data);
            stack_push(stack, stack_data(P_E, P_E));
            return 1;
        case 64: // division
            while (cnt < 5) {stack_pop(shelf); cnt++;}
            if (cnt != 5) goto cleanup;
            stack_pop(temps);
            stack_pop(temps);
            pair->value_type = D_FLOAT;

            printf("3p ");
            stack_push(temps, data);
            stack_push(stack, stack_data(P_E, P_E));
            return 1;
        case 65: // addition
            while (cnt < 5) {stack_pop(shelf); cnt++;}
            if (cnt != 5) goto cleanup;
            op_one = stack_pop(temps)->type;
            op_two = stack_pop(temps)->type;
            if (op_one == D_FLOAT || op_two == D_FLOAT) {
                pair->value_type = D_FLOAT;
            } else if (op_one == D_INT && op_two == D_INT) {
                pair->value_type = D_INT;
            }

            printf("4p ");
            stack_push(temps, data);
            stack_push(stack, stack_data(P_E, P_E));
            return 1;
        case 66: // subtraction
            while (cnt < 5) {stack_pop(shelf); cnt++;}
            if (cnt != 5) goto cleanup;
            op_one = stack_pop(temps)->type;
            op_two = stack_pop(temps)->type;
            if (op_one == D_FLOAT || op_two == D_FLOAT) {
                pair->value_type = D_FLOAT;
            } else if (op_one == D_INT && op_two == D_INT) {
                pair->value_type = D_INT;
            }

            printf("5p ");
            stack_push(temps, data);
            stack_push(stack, stack_data(P_E, P_E));
            return 1;
        case 67: // concatenation
            while (cnt < 5) {stack_pop(shelf); cnt++;}
            if (cnt != 5) goto cleanup;
            stack_pop(temps);
            stack_pop(temps);
            pair->value_type = D_STRING;

            printf("6p ");
            stack_push(temps, data);
            stack_push(stack, stack_data(P_E, P_E));
            return 1;
        case 59: // brackets
            while (cnt < 5) {stack_pop(shelf); cnt++;}
            if (cnt != 5) goto cleanup;
            printf("7p ");
            stack_push(stack, stack_data(P_E, P_E));
            return 1;
        case 71: // relations or a function
            if (fn) goto function;
            while (cnt < 5) {stack_pop(shelf); cnt++;}
            if (cnt != 5) goto cleanup;
            stack_pop(temps);
            stack_pop(temps);
            pair->value_type = D_BOOL;

            printf("8p ");
            stack_push(temps, data);
            stack_push(stack, stack_data(P_E, P_E));
            return 1;
        default:
            if (fn) goto function;
            break;
    }

    function:
    if (fn && res >= 71) {
        //symtable should be used here ideally to check for number of args
        int brackets = 0;
        int E = 0;
        TStack *reversal = NULL;
        stack_init(reversal);

        while (stack_top(shelf)->value != P_CLOSE) {
            data = stack_pop(shelf);
            if (data->value == P_LEFT_BRACKET) brackets--;
            else if (data->value == P_RIGHT_BRACKET) brackets++;
            else if (data->value == P_E) {
                E++;
                stack_push(reversal, data);
            }
        }
        if (brackets != 0) exit(BAD_SYNTAX);
        stack_pop(shelf); // pops last bracket

        // args number check
        if (E != last_fn->param_count) exit(BAD_TYPE_OR_RETURN);

        for (int i = 0; i < last_fn->param_count; i++) {
            if (stack_pop(reversal)->type != last_fn->params[i]) exit(BAD_TYPE_OR_RETURN);
        }

        printf("9p ");
        stack_push(stack, stack_data(P_E, P_E));
        return 1;
    } else {
        exit(BAD_SYNTAX);
    }

    cleanup:
    stack_dispose(shelf);
    return 0;
}

int precedence(TStack *stack, Token **token, bool *keep_token, bool *return_back) {
    stack_push(stack, stack_data(P_END, P_END));
    
    
    Token *lookahead = *token;
    bool end = false;
    unsigned int row, column;
    TStack *shelf = NULL;
    TStack *temps = NULL;
    shelf = stack_init(shelf);
    temps = stack_init(temps);
    while (true) {

        get_next_token(&lookahead, keep_token, return_back);
        
        if (lookahead->type == T_ERROR) goto bad_token;
        
        reduced:
        if (lookahead->type == T_LEFT_BRACE || lookahead->type == T_SEMICOLON) {
            end = true;
            //somehow return it back?
            *token = lookahead;
            *keep_token = true;
        }

        
        //only 1 skip needed?
        if (stack_top(stack)->value == P_E) stack_push(shelf, stack_pop(stack));

        //gets indexes for the table
        prec_index(NULL, &row,  (int)stack_top(stack)->value);
        if (!end) {
            prec_index(lookahead, &column, -1);
        } else {
            prec_index(NULL, &column, P_END);
        }
        
        unsigned int sym = PREC_TABLE[row][column];
        if (end && !sym) return 1;
        if (!sym) exit(BAD_SYNTAX);
        if (sym != P_EQUAL && !end) { // skips equal signs
            if (sym == P_CLOSE) {
                while (!stack_isEmpty(shelf)) {
                    stack_push(stack, stack_pop(shelf));
                }
            }
            stack_push(stack, stack_data((int) sym, (int) sym));
        }
        while (!stack_isEmpty(shelf)) {
            stack_push(stack, stack_pop(shelf));
        }
        if (end) stack_push(stack, stack_data(P_CLOSE, P_CLOSE));

        if (sym == P_CLOSE) {
            int res = reduce(stack, shelf, temps);
            if (!res || !stack_isEmpty(shelf)) exit(1); //TODO bad code
            goto reduced;
        }
        
        // if T_VAR is found -> P_I is pushed
        // if Rel operators are fund -> P_R is pushed
        // <(=)> is deleted, same for
        if (!end) {
            long long number_size = (long long)((ceil(log10(tmp_counter))+1)*sizeof(char));
            char *number = malloc(100);
            snprintf(number, number_size, "%d", tmp_counter);

            if (lookahead->type == T_IDENTIFIER) {
                htab_pair_t *pair = htab_find(temporary_tab, lookahead->value.identifier);
                TData *data = stack_data(P_FN, P_FN);
                data->bucket = pair;
                stack_push(stack, data);
                if (pair == NULL) {
                    exit(BAD_DEFINITION);
                }
            } else if (lookahead->type == T_FLOAT) {
                char tmp[100] = TEMP_VAR_PREFIX;
                strcat(tmp, number);

                htab_pair_t *pair = htab_find(temporary_tab, tmp);
                if (pair == NULL) {
                    tmp_counter++;
                    pair = htab_insert(temporary_tab, NULL, tmp);
                    pair->value.string = lookahead->value.string;
                    pair->value_type = D_FLOAT;
                }

                TData *data = stack_data(P_I, P_I);
                data->bucket = pair;

                stack_push(temps, data);
                stack_push(stack, stack_data(P_I, P_I));
            } else if (lookahead->type == T_INT) {
                char tmp[100] = TEMP_VAR_PREFIX;
                strcat(tmp, number);

                htab_pair_t *pair = htab_find(temporary_tab, tmp);
                if (pair == NULL) {
                    tmp_counter++;
                    pair = htab_insert(temporary_tab, NULL, tmp);
                    pair->value.number_int = lookahead->value.number_int;
                    pair->value_type = D_INT;
                }

                TData *data = stack_data(P_I, P_I);
                data->bucket = pair;

                stack_push(temps, data);
                stack_push(stack, stack_data(P_I, P_I));
            } else if (lookahead->type == T_STRING) {
                char tmp[100] = TEMP_VAR_PREFIX;
                strcat(tmp, number);

                htab_pair_t *pair = htab_find(temporary_tab, tmp);
                if (pair == NULL) {
                    tmp_counter++;
                    pair = htab_insert(temporary_tab, NULL, tmp);
                    pair->value.string = lookahead->value.string;
                    pair->value_type = D_STRING;
                }

                TData *data = stack_data(P_I, P_I);
                data->bucket = pair;

                stack_push(temps, data);
                stack_push(stack, stack_data(P_I, P_I));
            } else if (lookahead->type == T_VAR) {
                htab_pair_t *pair = htab_find(temporary_tab, lookahead->value.identifier);
                if (pair == NULL || pair->value_type == D_NONE) {
                    exit(BAD_UNDEFINED_VAR);
                }
                TData *data = stack_data(P_I, P_I);
                data->bucket = pair;

                stack_push(temps, data);
                stack_push(stack, stack_data(P_I, P_I));
            } else if (lookahead->type >= T_LESS && lookahead->type <= T_GREATER_EQUAL) {
                stack_push(stack, stack_data(P_R, P_R));
                if (relation_operator != 0) {
                    relation_operator = lookahead->type;
                } else {
                    exit(BAD_TYPE_COMPATIBILTY);
                }
            } else {
                stack_push(stack, stack_data((int) column, (int) column));
            }
        } else {
            while (reduce(stack, shelf, temps) == 1);
            if (reduce(stack, shelf, temps) != -1) {
                return 0;
            } else {
                return 1;
            }
        }
    }

    bad_token:
    exit(BAD_LEXEM);
}

int parse(void) {
    TStack *stack = NULL;
    stack = stack_init(stack);
    TStack *prec = NULL;
    prec = stack_init(prec);
    
    stack_push(stack, stack_data(T_EOF, T_TERM));
    stack_push(stack, stack_data(N_PROG, T_NONTERM));

    Token *token = malloc(sizeof(Token));
    if (token == NULL)  exit(1);
    tmp_token = malloc(sizeof(Token));
    if (tmp_token == NULL) exit(1);

    bool keep_prev_token = false;
    bool return_back = false;
    get_token(token);

    while(1) {
        if (stack_isEmpty(stack)) {
            break;
        }
        
        TData *top = stack_pop(stack);
        if (top->type == T_TERM) {
            if (top->value == token->type) { 

                /* add new tab to stack */
//                if (token->type == T_LEFT_BRACE) {
//                    htab_t *new_tab = htab_init(LOCALTAB_SIZE);
//                    TData *ptr = malloc(sizeof(TData));
//                    if (ptr != NULL) {
//                        ptr->value = 0;
//                        ptr->type = 0;
//                        ptr->htab = new_tab;
//                    }
//                    /* push newly created tab to stack */
//                    stack_push(local_tabs, ptr);
//                    temporary_tab = new_tab;
//                }
//                if (token->type == T_RIGHT_BRACE) {
//                    if (stack_isEmpty(local_tabs)) temporary_tab = glob_tab;
//                    else  temporary_tab = stack_pop(local_tabs)->htab;
//                }
                /* definition of a function */
                if (in_func != NULL) {
                    if (in_param_def && token->type == T_RIGHT_BRACKET) {
                        in_param_def = false;
                    }
                    else if (!in_param_def && token->type == T_TYPE) {
                        switch (token->value.keyword) {
                            case KW_INT:
                                in_func->return_type = D_INT;
                                break;
                            case KW_FLOAT:
                                in_func->return_type = D_FLOAT;
                                break;
                            case KW_STRING:
                                in_func->return_type = D_STRING;
                                break;
                            case KW_VOID:
                                in_func->return_type = D_VOID;
                                break;
                            default:
                                exit(1); // unknown data type
                        }
                        in_func = NULL;
                    }
                    else if (in_param_def && token->type == T_TYPE) {
                        in_func->param_count += 1;
                        
                        in_func->params = realloc(in_func->params, sizeof(Value) * in_func->param_count);
                        if (in_func->params == NULL) exit(1); // realloc failed
                        
                        switch (token->value.keyword) {
                            case KW_INT:
                                in_func->params[in_func->param_count - 1] = D_INT;
                                break;
                            case KW_FLOAT:
                                in_func->params[in_func->param_count - 1] = D_FLOAT;
                                break;
                            case KW_STRING:
                                in_func->params[in_func->param_count - 1] = D_STRING;
                                break;
                            case KW_VOID:
                                in_func->params[in_func->param_count - 1] = D_VOID;
                                break;
                            default:
                                exit(1); // unknown data type
                        }
                    }
                }
                
                get_next_token(&token, &keep_prev_token, &return_back);

                if (token->type == T_ASSIGN && tmp_token->type == T_VAR) {
                    in_assign = htab_insert(temporary_tab, tmp_token, tmp_token->value.identifier);
                    in_assign->value_type = D_NONE;
                }
            }
            else {
                fprintf(stderr, "terms not matching\n");
                exit(1);
            }
        }
        else if (top->type == T_KW) {
            if (token->type == T_KEYWORD && top->value == token->value.keyword) {
                get_next_token(&token, &keep_prev_token, &return_back);
                
                /* function definition - create new item in tab */
                if (tmp_token->value.keyword == KW_FUNCTION && token->type == T_IDENTIFIER) { 
                    in_func = htab_insert(temporary_tab, token, token->value.identifier);
                    in_func->param_count = 0;
                    in_func->params = NULL;
                    in_func->return_type = D_NONE;
                    in_param_def = true;
                }
                
            }
            else {
                fprintf(stderr, "terms not matching\n");
                exit(1);
            }

        }
        else if (top->type == T_NONTERM) {
            if (top->value == N_SMALL_ST && (token->type != T_ASSIGN && token->type != T_SEMICOLON)) {
                return_back = true;
            }
            if (top->value == N_EXPR) {

                /* CALL PRECEDENTIAL */
                keep_prev_token = true;
                int result = precedence(prec, &token, &keep_prev_token, &return_back);
                stack_dispose(prec);

                if (!result) exit(5); //TODO bad code
                get_next_token(&token, &keep_prev_token, &return_back);
                continue;
            }

            unsigned int col_idx, row_idx;
            if (token->type == T_KEYWORD) {
                col_idx = token->value.keyword;
            }
            else col_idx = token->type;

            row_idx = top->value;

            unsigned int val = LL_TABLE[row_idx][col_idx];
            if (val == 0) val = LL_TABLE[row_idx][EPS];

            printf("%d ", val);
            apply_rule(stack, val);
        }
        else {
            fprintf(stderr, "terms not matching\n");
            exit(1);
        }
    }
    printf("\n");
    stack_dispose(stack);
    return 0;
}

int main(void) {
    stream = fopen("test.php", "r");
    if (stream == NULL) exit(1);

    glob_tab = htab_init(GLOBTAB_SIZE);
    temporary_tab = glob_tab;
    local_tabs = stack_init(local_tabs);
    return parse();
}