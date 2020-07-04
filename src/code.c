#include <stdio.h>

#include "code.h"
#include "mem.h"

#define push(x) c_push(c, x)

static void err(code_t *c, const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void c_init(code_t *c) {
    c->n     = 0;
    c->cap   = 0;
    c->code  = NULL;
    c->k.n   = 0;
    c->k.cap = 0;
    c->k.v   = NULL;
}

void c_push(code_t *c, uint8_t b) {
    eval_resize(c->code, c->n, c->cap);
    c->code[c->n++] = b;
}

void c_free(code_t *c) {
    free(c);
    c_init(c);
}

// Emits a jump instruction and returns the location of the byte to be
// patched
int c_prep_jump(code_t *c, int jmp) {
    push(jmp);
    push(0x00); // Reserve byte
    return c->n - 1;
}

void c_patch(code_t *c, int loc) {
    c->code[loc] = (uint8_t) c->n;
}

static void c_pushk(code_t *c, int i) {
    switch (i) {
    case 0: push(OP_PUSHK0); break;
    case 1: push(OP_PUSHK1); break;
    case 2: push(OP_PUSHK2); break;
    default:
        push(OP_PUSHK);
        push((uint8_t) i);
        break;
    }
}

// Add a val_t literal to a code object's constant table, if
// necessary
void c_constant(code_t *c, token_t *tk) {

    // Search for existing duplicate literal
    for (int i = 0; i < c->k.n; i++) {
        switch (tk->kind) {
        case TK_FLT:
            if (c->k.v[i]->type != TYPE_FLT)
                break;
            else if (tk->lexeme.f == c->k.v[i]->u.f) {
                c_pushk(c, i);
                return;
            }
            break;
        case TK_INT:
            if (c->k.v[i]->type != TYPE_INT)
                break;
            else if (tk->lexeme.i == c->k.v[i]->u.i) {
                c_pushk(c, i);
                return;
            }
            break;
        case TK_STR:
            if (c->k.v[i]->type != TYPE_STR)
                break;
            else if (tk->lexeme.s->hash == c->k.v[i]->u.s->hash) {
                c_pushk(c, i);
                return;
            }
            break;
        default: break;
        }
    }

    // Provided literal does not already exist in constant table
    val_t *v;
    switch (tk->kind) {
    case TK_FLT:
        v = v_newflt(tk->lexeme.f);
        break;
    case TK_INT: {
        int_t i = tk->lexeme.i;
        switch (i) {
        case 0: push(OP_PUSH0); return;
        case 1: push(OP_PUSH1); return;
        case 2: push(OP_PUSH2); return;
        default:
            if (i >= 3 && i <= 255) {
            push(OP_PUSHI);
            push((uint8_t) i);
            return;
        } else {
            v = v_newint(i);
        }
            break;
        }
        break;
    }
    case TK_STR:
        v = v_newstr(tk->lexeme.s);
        break;
    default: break;
    }

    eval_resize(c->k.v, c->k.n, c->k.cap);
    c->k.v[c->k.n++] = v;
    if (c->k.n > 255)
        err(c, "Exceeded max number of unique literals");
    c_pushk(c, c->k.n - 1);
}

static void c_pushs(code_t *c, int i) {
    switch (i) {
    case 0: push(OP_PUSHS0); break;
    case 1: push(OP_PUSHS1); break;
    case 2: push(OP_PUSHS2); break;
    default:
        push(OP_PUSHS);
        push((uint8_t) i);
        break;
    }
}

void c_symbol(code_t *c, token_t *tk) {
    // Search for existing symbol
    for (int i = 0; i < c->k.n; i++) {
        if (c->k.v[i]->type != TYPE_STR)
            break;
        else if (tk->lexeme.s->hash == c->k.v[i]->u.s->hash) {
            c_pushs(c, i);
            return;
        }
    }
    val_t *v;
    v = v_newstr(tk->lexeme.s);
    eval_resize(c->k.v, c->k.n, c->k.cap);
    c->k.v[c->k.n++] = v;
    if (c->k.n > 255)
        err(c, "Exceeded max number of unique literals");
    c_pushs(c, c->k.n - 1);
}

void c_infix(code_t *c, int op) {
    switch (op) {
    case '+':       push(OP_ADD);    break;
    case '-':       push(OP_SUB);    break;
    case '*':       push(OP_MUL);    break;
    case '/':       push(OP_DIV);    break;
    case '%':       push(OP_MOD);    break;
    case '>':       push(OP_GT);     break;
    case '<':       push(OP_LT);     break;
    case '=':       push(OP_SET);    break;
    case '&':       push(OP_AND);    break;
    case '|':       push(OP_OR);     break;
    case '^':       push(OP_XOR);    break;
    case TK_SHL:    push(OP_SHL);    break;
    case TK_SHR:    push(OP_SHR);    break;
    case TK_POW:    push(OP_POW);    break;
    case TK_CAT:    push(OP_CAT);    break;
    case TK_GE:     push(OP_GE);     break;
    case TK_LE:     push(OP_LE);     break;
    case TK_EQ:     push(OP_EQ);     break;
    case TK_NE:     push(OP_NE);     break;
    case TK_ADDASG: push(OP_ADDASG); break;
    case TK_ANDASG: push(OP_ANDASG); break;
    case TK_DIVASG: push(OP_DIVASG); break;
    case TK_MODASG: push(OP_MODASG); break;
    case TK_MULASG: push(OP_MULASG); break;
    case TK_ORASG:  push(OP_ORASG);  break;
    case TK_SUBASG: push(OP_SUBASG); break;
    case TK_XORASG: push(OP_XORASG); break;
    case TK_CATASG: push(OP_CATASG); break;
    case TK_POWASG: push(OP_POWASG); break;
    case TK_SHLASG: push(OP_SHLASG); break;
    case TK_SHRASG: push(OP_SHRASG); break;
    default: break;
    }
}

void c_prefix(code_t *c, int op) {
    switch (op) {
    case '!':    push(OP_LNOT);   break;
    case '#':    push(OP_LEN);    break;
    case '+':    push(OP_NUM);    break;
    case '-':    push(OP_NEG);    break;
    case '~':    push(OP_NOT);    break;
    case TK_INC: push(OP_PREINC); break;
    case TK_DEC: push(OP_PREDEC); break;
    default: break;
    }
}

void c_postfix(code_t *c, int op) {
    switch (op) {
    case TK_INC: push(OP_POSTINC); break;
    case TK_DEC: push(OP_POSTDEC); break;
    default: break;
    }
}

#undef push
