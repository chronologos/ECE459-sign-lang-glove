typedef struct {
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    int dp;
} DisplayState;

void set(char x, DisplayState* ds) {
    switch (x) {
        case '0':
            ds->a = 1;
            ds->b = 1;
            ds->c = 1;
            ds->d = 1;
            ds->e = 1;
            ds->f = 1;
            ds->g = 0;
            ds->dp = 0;
            break;
        case '1':
            ds->a = 0;
            ds->b = 1;
            ds->c = 1;
            ds->d = 0;
            ds->e = 0;
            ds->f = 0;
            ds->g = 0;
            ds->dp = 0;
            break;
        case '2':
            ds->a = 1;
            ds->b = 1;
            ds->c = 0;
            ds->d = 1;
            ds->e = 1;
            ds->f = 0;
            ds->g = 1;
            ds->dp = 0;
            break;
        case '3':
            ds->a = 1;
            ds->b = 1;
            ds->c = 1;
            ds->d = 1;
            ds->e = 0;
            ds->f = 0;
            ds->g = 1;
            ds->dp = 0;
            break;
        case '4':
            ds->a = 0;
            ds->b = 1;
            ds->c = 1;
            ds->d = 0;
            ds->e = 0;
            ds->f = 1;
            ds->g = 1;
            ds->dp = 0;
            break;
        case '5':
            ds->a = 1;
            ds->b = 0;
            ds->c = 1;
            ds->d = 1;
            ds->e = 0;
            ds->f = 1;
            ds->g = 1;
            ds->dp = 0;
            break;
        case '6':
            ds->a = 1;
            ds->b = 0;
            ds->c = 1;
            ds->d = 1;
            ds->e = 1;
            ds->f = 1;
            ds->g = 1;
            ds->dp = 0;
            break;
        case '7':
            ds->a = 1;
            ds->b = 1;
            ds->c = 1;
            ds->d = 0;
            ds->e = 0;
            ds->f = 0;
            ds->g = 0;
            ds->dp = 0;
            break;
        case '8':
            ds->a = 1;
            ds->b = 1;
            ds->c = 1;
            ds->d = 1;
            ds->e = 1;
            ds->f = 1;
            ds->g = 1;
            ds->dp = 0;
            break;
        case '9':
            ds->a = 1;
            ds->b = 1;
            ds->c = 1;
            ds->d = 1;
            ds->e = 0;
            ds->f = 1;
            ds->g = 1;
            ds->dp = 0;
            break;
        case '.':
            ds->a = 0;
            ds->b = 0;
            ds->c = 0;
            ds->d = 0;
            ds->e = 0;
            ds->f = 0;
            ds->g = 0;
            ds->dp = 1;
            break;
        case '-':
            ds->a = 0;
            ds->b = 0;
            ds->c = 0;
            ds->d = 1;
            ds->e = 0;
            ds->f = 0;
            ds->g = 0;
            ds->dp = 0;
            break;
        case 'e':
            ds->a = 1;
            ds->b = 1;
            ds->c = 0;
            ds->d = 1;
            ds->e = 1;
            ds->f = 1;
            ds->g = 1;
            ds->dp = 0;
            break;
        case ' ':
            ds->a = 0;
            ds->b = 0;
            ds->c = 0;
            ds->d = 0;
            ds->e = 0;
            ds->f = 0;
            ds->g = 0;
            ds->dp = 0;
            break;
        default:
            break;
    }
}


