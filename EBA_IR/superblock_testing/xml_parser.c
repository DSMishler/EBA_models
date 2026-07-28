/* xmlparser, made by claude AI */
/*
 * xmlparse.c — minimal recursive-descent XML parser
 *
 * Handles: nested elements, attributes, text content, self-closing tags.
 * Does NOT handle: CDATA, comments, entities (&amp; etc.), namespaces,
 * processing instructions, DOCTYPE. Assumes well-formed input.
 *
 * Build:  cc -o xmlparse xmlparse.c
 * Usage:  ./xmlparse file.xml
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Attr {
    char *name;
    char *value;
    struct Attr *next;
} Attr;

typedef struct Node {
    char *tag;
    Attr *attrs;
    char *text;          /* text content, if any (trimmed) */
    struct Node *first_child;
    struct Node *next_sibling;
} Node;

/* ---- parser state ---- */
typedef struct {
    const char *src;
    size_t pos;
    size_t len;
} Parser;

static void skip_ws(Parser *p) {
    while (p->pos < p->len && isspace((unsigned char)p->src[p->pos]))
        p->pos++;
}

static int peek(Parser *p) {
    return p->pos < p->len ? (unsigned char)p->src[p->pos] : -1;
}

static int match(Parser *p, char c) {
    if (peek(p) == c) { p->pos++; return 1; }
    return 0;
}

static char *xstrndup(const char *s, size_t n) {
    char *r = malloc(n + 1);
    memcpy(r, s, n);
    r[n] = '\0';
    return r;
}

/* parse an XML Name: [A-Za-z_:][A-Za-z0-9_:.-]* */
static char *parse_name(Parser *p) {
    size_t start = p->pos;
    if (!(isalpha((unsigned char)peek(p)) || peek(p) == '_' || peek(p) == ':'))
        return NULL;
    p->pos++;
    while (p->pos < p->len) {
        int c = peek(p);
        if (isalnum((unsigned char)c) || c == '_' || c == ':' || c == '.' || c == '-')
            p->pos++;
        else
            break;
    }
    return xstrndup(p->src + start, p->pos - start);
}

/* parse a quoted attribute value: "..." or '...' */
static char *parse_quoted(Parser *p) {
    char quote = peek(p);
    if (quote != '"' && quote != '\'') return NULL;
    p->pos++;
    size_t start = p->pos;
    while (p->pos < p->len && peek(p) != quote)
        p->pos++;
    char *val = xstrndup(p->src + start, p->pos - start);
    match(p, quote);
    return val;
}

static Attr *parse_attrs(Parser *p) {
    Attr *head = NULL, *tail = NULL;
    for (;;) {
        skip_ws(p);
        int c = peek(p);
        if (c == '>' || c == '/' || c == -1) break;

        char *name = parse_name(p);
        if (!name) break; /* malformed; bail out of attr loop */

        skip_ws(p);
        char *value = NULL;
        if (match(p, '=')) {
            skip_ws(p);
            value = parse_quoted(p);
        }

        Attr *a = malloc(sizeof(Attr));
        a->name = name;
        a->value = value ? value : xstrndup("", 0);
        a->next = NULL;
        if (!head) head = a; else tail->next = a;
        tail = a;
    }
    return head;
}

/* trim leading/trailing whitespace, return NULL if empty */
static char *trim_text(const char *s, size_t n) {
    size_t start = 0, end = n;
    while (start < end && isspace((unsigned char)s[start])) start++;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start == end) return NULL;
    return xstrndup(s + start, end - start);
}

static Node *parse_element(Parser *p);

/* parse a sequence of sibling nodes/text until we hit "</" or end */
static Node *parse_children(Parser *p, char **out_text) {
    Node *head = NULL, *tail = NULL;
    *out_text = NULL;

    for (;;) {
        size_t text_start = p->pos;
        while (p->pos < p->len && peek(p) != '<')
            p->pos++;

        if (p->pos > text_start) {
            char *t = trim_text(p->src + text_start, p->pos - text_start);
            if (t) {
                if (*out_text) free(t); /* keep first non-empty chunk only, simple approach */
                else *out_text = t;
                if (t != *out_text) free(t);
            }
        }

        if (p->pos >= p->len) break;

        if (p->src[p->pos] == '<' && p->pos + 1 < p->len && p->src[p->pos + 1] == '/') {
            break; /* closing tag of parent — let caller consume it */
        }

        Node *child = parse_element(p);
        if (!child) break;
        if (!head) head = child; else tail->next_sibling = child;
        tail = child;
    }
    return head;
}

static Node *parse_element(Parser *p) {
    if (!match(p, '<')) return NULL;
    char *tag = parse_name(p);
    if (!tag) { p->pos--; return NULL; }

    Node *node = calloc(1, sizeof(Node));
    node->tag = tag;
    node->attrs = parse_attrs(p);
    skip_ws(p);

    if (match(p, '/')) {           /* self-closing: <tag/> */
        match(p, '>');
        return node;
    }

    if (!match(p, '>')) {          /* malformed */
        return node;
    }

    node->first_child = parse_children(p, &node->text);

    /* expect closing tag </tag> */
    if (match(p, '<') && match(p, '/')) {
        char *close_tag = parse_name(p);
        skip_ws(p);
        match(p, '>');
        free(close_tag); /* not validated against open tag for simplicity */
    }

    return node;
}

static Node *xml_parse(const char *src) {
    Parser p = { src, 0, strlen(src) };
    skip_ws(&p);
    return parse_element(&p);
}

/* ---- pretty printer, for demo/testing ---- */
static void print_node(Node *n, int depth) {
    for (Node *cur = n; cur; cur = cur->next_sibling) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("<%s", cur->tag);
        for (Attr *a = cur->attrs; a; a = a->next)
            printf(" %s=\"%s\"", a->name, a->value);
        printf(">");
        if (cur->text) printf(" \"%s\"", cur->text);
        printf("\n");
        if (cur->first_child) print_node(cur->first_child, depth + 1);
    }
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); exit(1); }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s file.xml\n", argv[0]);
        return 1;
    }
    char *src = read_file(argv[1]);
    Node *root = xml_parse(src);
    if (!root) {
        fprintf(stderr, "parse failed\n");
        return 1;
    }
    print_node(root, 0);
    return 0;
}
