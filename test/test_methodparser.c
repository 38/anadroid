#include <dalvik/dalvik_method.h>
#include <anadroid.h>
int main(int argc, char** argv)
{
    anadroid_init();

    FILE* fp = fopen(argv[1], "r");
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp) + 1;
    char *buf = (char*)malloc(len);
    fseek(fp, 0, SEEK_SET);
    int nbytes;
    if((nbytes = fread(buf, 1, len, fp)) < 0)
    {
        LOG_ERROR("Can't read file");
        return 0;
    }
    buf[nbytes] = 0;

    fclose(fp);
    const char *ptr;
    for(ptr = buf; ptr != NULL && ptr[0] != 0;)
    {
        sexpression_t* sexp;
        if(NULL == (ptr = sexp_parse(ptr, &sexp)))
        {
            LOG_ERROR("Can't parse S-Expression");
            return 0;
        }
        if(SEXP_NIL == sexp) continue;
        if(NULL == dalvik_class_from_sexp(sexp))
        {
            LOG_ERROR("Can't parse class definitions");
            return 0;
        }
        sexp_free(sexp);
    }
    free(buf);
    anadroid_finalize();
    return 0;
}
