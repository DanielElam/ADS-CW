
static const size_t str_builder_min_size = 32;

struct str_builder
{
    char* buffer;
    int count;
    int capacity;
};

void str_builder_init(struct str_builder* sb)
{
    sb->buffer = malloc(str_builder_min_size);
    *sb->buffer = '\0';
    sb->capacity = str_builder_min_size;
    sb->count = 0;
}

void str_builder_ensureCapacity(struct str_builder* sb, int increment)
{
    if (sb->count + increment > sb->capacity)
    {
        const int newCapacity = (sb->count + increment * 2) + 1;
        sb->buffer = realloc(sb->buffer, sizeof(char) * newCapacity);
    }
}

void str_builder_addChar(struct str_builder* sb, unsigned char c)
{
    if (sb == NULL)
        return;
    str_builder_ensureCapacity(sb, 1);
    sb->buffer[sb->count] = c;
    sb->buffer[sb->count + 1] = '\0';
    sb->count++;
}

void str_builder_addString(struct str_builder* sb, unsigned char* str, int strLen)
{
    if (sb == NULL || str == NULL || *str == '\0')
        return;

    if (strLen == 0)
        strLen = strlen(str);

    str_builder_ensureCapacity(sb, strLen);
    memmove(sb->buffer + sb->count, str, strLen);
    sb->count += strLen;
    sb->buffer[sb->count] = '\0';
}

char* str_builder_getString(struct str_builder* list)
{
    return list->buffer;
}
