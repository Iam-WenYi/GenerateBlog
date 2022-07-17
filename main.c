/*
    main.c
    生成一个喜欢的博客
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PrintDebugMessage(message) printf("Debug message:\n\tFunction : %s\n\tLine: %d : %s\n", __func__, __LINE__, message);
#define SITE_CONFIG_DIRECTORY ".config"
#define SITE_CONFIG_FILE SITE_CONFIG_DIRECTORY"/config.ini"
#define SITE_THEME_DIRECTORY SITE_CONFIG_DIRECTORY"/theme"
#define SITE_THEME_INDEX_PAGE SITE_CONFIG_DIRECTORY"/theme/index.html"
#define SITE_THEME_ARTICLE_PAGE SITE_CONFIG_DIRECTORY"/theme/article.html"
#define SITE_NAME "{ sitename }"
#define SITE_MOTTO "{ sitemotto }"
#define SITE_THEME_PAGE_ARTICLE_TITLE "{ articletitle }"
#define SITE_THEME_PAGE_ARTICLE_CONTENT "{ articlecontent }"
#define SITE_THEME_PAGE_ARTICLE_LIST "{ articlelist }"
#define MAX_LENGTH 512
#define MAX_ARTICLE_LENGTH 96
#define MAX_CONTENT_LENGTH 96
#define DATE_LENGTH 21

typedef struct SiteConfig {
    char name[MAX_LENGTH];
    char motto[MAX_LENGTH];
    char *theme_index;
    char *theme_article;
}SiteConfig;
SiteConfig config = {};

/*
    PrintHelpMessage
    打印帮助消息
*/
int PrintHelpMessage()
{
    puts("GenerateBlog");
    puts("    Generate a blog");
    puts("    --help");
    puts("      Print the help message");
    puts("    --version");
    puts("      Print the version message");
    puts("    init [source_directory]");
    puts("       Init a new site");
    puts("    compile [source_directory] -o [output_directory]");
    puts("      Generate a blog for this directory");
    return 0;
}
/*
    PrintVersion
    打印版本信息
*/
int PrintVersion()
{
    puts("Version 2022.07.12");
    return 0;
}
/*
    FileContent
    获取文件的全部内容
*/
char* FileContent(const char *filename)
{
    FILE *fp = NULL;
    char *result = NULL;
    int filesize = 0;

    fp = fopen(filename, "r");
    if (NULL == fp)
        return NULL;
    
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    result = (char*)calloc(filesize, sizeof(char));
    if (NULL == result)
    {
        fclose(fp);
        return NULL;
    }

    fread(result, 1, filesize, fp);
    fclose(fp);

    return result;
}
/*
    LoadConfig
    加载配置文件
*/
SiteConfig LoadConfig(const char *source_dir)
{
    SiteConfig result = {};
    char filename[MAX_LENGTH] = "";
    FILE *fp = NULL;

    sprintf(filename, "%s/%s", source_dir, SITE_CONFIG_FILE);
    
    fp = fopen(filename, "r");
    if (NULL == fp)
    {
        puts("Error: Couldn't open config file");
        exit(-1);
    }

    fscanf(fp, "sitename => %s\n", result.name);
    fscanf(fp, "sitemotto => %s\n", result.motto);
    fclose(fp);
    
    sprintf(filename, "%s/%s", source_dir, SITE_THEME_INDEX_PAGE);
    if (!(result.theme_index = FileContent(filename)))
    {
        puts("Error: Couldn't open the index theme file");
        exit(-1);
    }

    sprintf(filename, "%s/%s", source_dir, SITE_THEME_ARTICLE_PAGE);
    if (!(result.theme_article = FileContent(filename)))
    {
        puts("Error: Couldn't open the article theme file");
        exit(-1);
    }

    return result;
}
/*
    WriteFile
    向文件写入内容
*/
int WriteFile(const char *filename, const void *data)
{
    FILE *fp = NULL;

    fp = fopen(filename, "w");
    if (NULL == fp)
        return -1;

    fwrite(data, 1, strlen(data), fp);
    fclose(fp);
    return 0;
}
/*
    ListSiteFiles
    遍历站点文件夹中的文件
*/
int ListSiteFiles(const char *dirname, const char *dest_dirname, int (*Handle)(const char *source_directory, const char *filename, const char *dest_directory))
{
    DIR *dir = NULL;
    struct dirent *directory = NULL;

    dir = opendir(dirname);
    if (NULL == dir)
        return -1;

    while (directory = readdir(dir))
    {
        if ('.' == directory -> d_name[0] || DT_REG != directory -> d_type)
            continue;
        Handle(dirname, directory -> d_name, dest_dirname);
    }

    closedir(dir);
    return 0;
}
/*
    GenerateOutputFileName
    生成输出文件名称(完整路径)
*/
char* GenerateOutputFileName(const char *source_directory, const char *source_filename, const char *dest_directory)
{
    char *result = NULL;
    char *begin_position = source_filename, *end_position = 0;
    end_position = strrchr(source_filename, '.');

    result = (char*)calloc(1, strlen(dest_directory) + sizeof("/") + (end_position - begin_position) + sizeof("html"));
    if (NULL == result)
        return NULL;

    memcpy(result, dest_directory, strlen(dest_directory));
    strcat(result, "/");
    strncat(result, source_filename, (end_position - begin_position));
    strcat(result, ".html");

    return result;
}
/*
    CompileMarkdown
    编译 Markdown 文件为HTML代码
*/
int CompileMarkdown(const char* source_path, const char *source_file, const char *output_filename)
{
    char command[MAX_LENGTH] = {};

    sprintf(command, "./md2html --ftables --ftasklists --funderline --fwiki-links --fpermissive-url-autolinks --fpermissive-www-autolinks --fpermissive-email-autolinks '%s/%s' -o '%s'", source_path, source_file, output_filename);
    
    return system(command);
}
/*
    strrep
    字符串替换函数
*/
char* strrep(const char *source, const char *needle, const char *replacement)
{
    char *result = NULL;
    char *temp = NULL, *temp_2 = NULL;
    char *begin = NULL;
    int needle_len = 0, replacement_len = 0;
    int size = 0;
    int diff = 0;
    int count = 0;
    
    needle_len = strlen(needle);
    replacement_len = strlen(replacement);
    temp = source;
    size = strlen(source);
    diff = replacement_len - needle_len;
    while (temp = strstr(temp, needle))
    {
        count++;
        temp++;
    }
    size += count * diff;

    result = (char*)calloc(size, sizeof(char));
    if (NULL == result)
        return NULL;

    begin = source;
    temp = source;
    temp_2 = result;
    while (temp = strstr(temp, needle))
    {
        memcpy(result, begin, temp - begin);
        result += (temp - begin);
        strcat(result, replacement);
        result += replacement_len;
        begin = temp + needle_len;
        temp += needle_len;
    }
    strcat(result, begin);
    result = temp_2;
    return result;
}
/*
    GetArticleTitleByFileName
    根据文件名获取文章标题
*/
char* GetArticleTitleByFileName(const char *filename)
{
    char *result = NULL;
    char *begin = NULL, *end = NULL;
    begin = strrchr(filename, '/') + 1;
    if (1 == begin || begin < filename)
        begin = filename;

    end = strrchr(filename, '.');
    if (0 == end)
        return NULL;
    result = (char*)calloc(1, end - begin);

    if (NULL == result)
        return NULL;
    memcpy(result, begin, end - begin);
    return result;
}
/*
    BuildHTML
    由片段性HTML，结合主题文件，构建完整的HTML文件
*/
int BuildHTML(const char *output_filename)
{
    char *filecontent = NULL;
    char *output = NULL;
    char *temp = NULL;
    char *article_title = NULL;

    article_title = GetArticleTitleByFileName(output_filename);
    if (NULL == article_title)
        return NULL;

    filecontent = FileContent(output_filename);
    if (NULL == filecontent)
    {
        free(article_title);
        return NULL;
    }
    output = strrep(config.theme_article, SITE_NAME, config.name);
    temp = output;
    temp = strrep(output, SITE_THEME_PAGE_ARTICLE_CONTENT, filecontent);
    free(filecontent);
    free(output);
    output = temp;
    temp = strrep(output, SITE_THEME_PAGE_ARTICLE_TITLE, article_title);
    free(output);
    output = temp;
    WriteFile(output_filename, output);
    free(output);

    return 0;
}
/*
    CompileFile
    编译单独的 Markdown 文件为 HTML 文件
*/
int CompileFile(const char* source_path, const char *source_file, const char *output_path)
{
    char *output_filename = NULL;
    output_filename = GenerateOutputFileName(source_path, source_file, output_path);
    CompileMarkdown(source_path, source_file, output_filename);
    BuildHTML(output_filename);
    free(output_filename);
    return 0;
}
/*
    GetBriefArticle
    提取部分文章内容作为“前情提要”
*/
char* GetBriefArticle(const char *article)
{
    int len = 0;
    char *result = NULL;
    result = (char*)calloc(sizeof(char), MAX_CONTENT_LENGTH);
    if (NULL == result)
        return NULL;
    while (*article != '\n')
    {
        *(result + len) = *article;
        article++;
        len++;
    }
    return result;
}
/*
    BuildIndex
    构建站点主页
*/
int BuildIndex(const char *source_path, const char *output_dir)
{
    FILE *fp = NULL;
    char filename[1024] = {};
    char *temp = NULL, *temp_2 = NULL, *temp_3 = NULL;
    char *output = NULL;
    DIR *dir = NULL;
    int count = 0;
    struct dirent *directory = NULL;
    sprintf(filename, "%s/index.html", output_dir);
    fp = fopen(filename, "w");
    if (NULL == fp)
        return -1;

    dir = opendir(source_path);
    if (NULL == dir)
    {
        fclose(fp);
        return -1;
    }
    while (directory = readdir(dir))
    {
        if ('.' == directory -> d_name[0] || DT_REG != directory -> d_type)
            continue;
        sprintf(filename, "%s/%s", source_path, directory -> d_name);
        temp = FileContent(filename);
        fprintf(fp, "<div><h4 style = \"margin-left: -15px\"><a href = \"./%s.html\" class=\"nav-link text-dark\">", temp_3 = GetArticleTitleByFileName(directory -> d_name));
        free(temp_3);
        fprintf(fp, "%s",temp_2 = GetArticleTitleByFileName(directory -> d_name));
        free(temp_2);
        fprintf(fp, "</a></h4><p>");
        fprintf(fp, "%s", temp_2 = GetBriefArticle(temp));
        free(temp_2);
        fprintf(fp, "</p><hr></div>");
        free(temp);
        temp = NULL;
    }
    closedir(dir);
    fclose(fp);

    sprintf(filename, "%s/index.html", output_dir);
    temp = FileContent(filename);
    output = strrep(config.theme_index, SITE_THEME_PAGE_ARTICLE_LIST, temp);
    free(temp);
    temp = strrep(output, SITE_NAME, config.name);
    free(output);
    output = strrep(temp, SITE_MOTTO, config.motto);
    free(temp);
    WriteFile(filename, output);
    free(output);

    return 0;
}
/*
    CompileSite
    构建博客网站
*/
int CompileSite(const char *source_dir, const char *output_dir)
{
    config = LoadConfig(source_dir);
    mkdir(output_dir, 0777);
    ListSiteFiles(source_dir, output_dir, CompileFile);
    BuildIndex(source_dir, output_dir);
    free(config.theme_index);
    free(config.theme_article);
    return 0;
}
/*
    InitSite
    初始化博客网站
*/
int InitSite(const char *source_dir)
{
    char command[MAX_LENGTH] = {};
    sprintf(command, "cp -r ./.config %s/.config", source_dir);
    return system(command);
}
int main(int argc, char *argv[])
{
    if (argc <= 1)
        return PrintHelpMessage();
    else if (!strcmp("--help", argv[1]))
        return PrintHelpMessage();
    else if (!strcmp("--version", argv[1]))
        return PrintVersion();
    else if (!strcmp("init", argv[1]))
        if (argc != 3)
            return PrintHelpMessage();
        else
            return InitSite(argv[2]);
    else if (!strcmp("compile", argv[1]))
        if (argc != 5)
            return PrintHelpMessage();
        else
            return CompileSite(argv[2], argv[4]);
    else
        return PrintHelpMessage();
    return 0;
}