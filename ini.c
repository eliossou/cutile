// Supports single-line comments using ';' character: whole line after ';' is skipped.

#ifndef CUT_INI
    #define CUT_INI

    #include "base.c"
    #include "memory.c"

    typedef struct Cut_Ini_Field {
        cut_u32 name_start;       // Where the name starts.
        cut_u32 name_end;         // Where the name ends, before '='.
        cut_u32 val_start;        // Where the value starts, after '='.
        cut_u32 val_end;          // Where the value ends, before new line.
    } Cut_Ini_Field;

    typedef struct Cut_Ini_Section {
        cut_u32 decl_start;       // '['
        cut_u32 decl_end;         // ']'
        cut_u32 name;             // Name of the section: after '['.
        cut_u32 name_size;        // Size in bytes of the section name.
        cut_u32 content_start;    // After section declaration.
        cut_u32 content_end;      // One byte before next section declaration or end of file.

        struct {
            cut_u32 count;
            cut_u32 capacity;

            Cut_Ini_Field *data;
        } fields;
    } Cut_Ini_Section;

    typedef struct Cut_Ini_Error {
        cut_u8arrview msg;
        cut_u32 line;
    } Cut_Ini_Error;

    #ifndef     CUT_PARSE_INI_SECTION_FIELDS_INIT_CAPACITY
        #define CUT_PARSE_INI_SECTION_FIELDS_INIT_CAPACITY 4
    #endif

    #ifndef     CUT_PARSE_INI_SECTIONS_INIT_CAPACITY
        #define CUT_PARSE_INI_SECTIONS_INIT_CAPACITY 4
    #endif

    typedef struct Cut_Parse_Ini_Result {
        Cut_Ini_Error   error;
        Cut_Ini_Section global_section;

        cut_u8arrview ini_data;

        struct {
            cut_u32 count;
            cut_u32 capacity;

            Cut_Ini_Section *data;
        } sections;

        Cut_Mem_Allocator mem_allocator;
    } Cut_Parse_Ini_Result;

    Cut_Parse_Ini_Result cut_ini_parse(cut_u8arrview ini_data, Cut_Mem_Allocator mem_allocator);

    void cut_ini_destroy_parsed_data(Cut_Parse_Ini_Result *parse_ini_result);

    typedef struct Cut_Ini_Get_Entry_Result {
        int found;
        cut_u32 value_start;
        cut_u32 value_size;
    } Cut_Ini_Get_Entry_Result;

    Cut_Ini_Get_Entry_Result cut_ini_get_global_entry_value(Cut_Parse_Ini_Result *parse_ini_result, cut_u8arrview entry_name);
    Cut_Ini_Get_Entry_Result cut_ini_get_entry_value(Cut_Parse_Ini_Result *parse_ini_result, cut_u8arrview section_name, cut_u8arrview entry_name);

    int cut_ini_get_field(cut_u8arrview ini_data, cut_u8arrview section_name, cut_u8arrview entry_name, Cut_Ini_Field *out);
    int cut_ini_get_global_field(cut_u8arrview ini_data, cut_u8arrview entry_name, Cut_Ini_Field *out);

    #if defined(CUT_INI_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define Ini_Field Cut_Ini_Field
        #define Ini_Section Cut_Ini_Section
        #define Ini_Error Cut_Ini_Error
        #define Parse_Ini_Result Cut_Parse_Ini_Result
        #define ini_parse cut_ini_parse
        #define ini_destroy_parsed_data cut_ini_destroy_parsed_data
        #define Ini_Get_Entry_Result Cut_Ini_Get_Entry_Result
        #define ini_get_global_entry_value cut_ini_get_global_entry_value
        #define ini_get_entry_value cut_ini_get_entry_value
        #define ini_get_field cut_ini_get_field
        #define ini_get_global_field cut_ini_get_global_field
    #endif
#endif

#if !defined(CUT_INI_IMPL_INCLUDED) && (defined(CUT_INI_IMPL) || defined(CUT_IMPL))
    #define CUT_INI_IMPL_INCLUDED

    typedef struct Cut_Ini_Parser_State {
        cut_u8 *data;
        cut_u32 data_size;
        cut_u32 index;
        cut_u32 line;
    } Cut_Ini_Parser_State;

    typedef enum Cut_Ini_Parser_Token_Kind {
        cut_ini_parser_token_kind_separator,
        cut_ini_parser_token_kind_op,
        cut_ini_parser_token_kind_name_or_value,
        cut_ini_parser_token_kind_end,
        cut_ini_parser_token_kind_unknown
    } Cut_Ini_Parser_Token_Kind;

    typedef struct Cut_Ini_Parser_Token {
        Cut_Ini_Parser_Token_Kind kind;
        cut_u32 start;
        cut_u32 end;
        cut_u32 line;
    } Cut_Ini_Parser_Token;

    cut_internal
    Cut_Ini_Parser_Token cut_ini_parser_get_next_token(Cut_Ini_Parser_State *state)
    {
        Cut_Ini_Parser_Token tok;
        cut_u8 ch;

        begin:

        // Extracts characters until one is relevant.
        while (1) {
            if (state->index >= state->data_size)
                goto end_of_data;

            ch = state->data[state->index];

            if (ch == ' ' || ch == '\t' || ch == '\r') {
                state->index++;
                continue;
            } else if (ch == '\n') {
                state->index++;
                state->line++;
            } else {
                break;
            }
        }

        // Comment
        if (ch == ';') {
            while (1) {
                if (state->index >= state->data_size)
                    goto end_of_data;

                ch = state->data[state->index++];

                if (ch == '\n') { // End of comment.
                    state->line++;
                    goto begin;
                }
            }
        }

        tok.line = state->line;

        // Section separators.
        if (ch == '[' || ch == ']') {
            tok.kind = cut_ini_parser_token_kind_separator;
            tok.start = state->index;
            tok.end = state->index;
            state->index++;
        }

        // Assignment operator.
        else if (ch == '=') {
            tok.kind = cut_ini_parser_token_kind_op;
            tok.start = state->index;
            tok.end = state->index;
            state->index++;
        }

        // Section name or entry name or entry value.
        else if (ch >= 32 && ch <= 126 && ch != '=') {
            tok.kind = cut_ini_parser_token_kind_name_or_value;
            tok.start = state->index;
            tok.end = state->index;
            state->index++;
            while (state->index < state->data_size) {
                ch = state->data[state->index];

                // Allowed characters:
                if (ch >= 32 && ch <= 126 && ch != '=' && ch != '[' && ch != ']') {
                    tok.end++;
                    state->index++;
                } else {
                    break;
                }
            }
        }

        // Unknown token... Something is wrong!
        else
            tok.kind = cut_ini_parser_token_kind_unknown;

        return tok;

        end_of_data: {
            tok.kind = cut_ini_parser_token_kind_end;
            tok.line = state->line;
            return tok;
        }
    }

    cut_internal void cut_ini_parse_entry(
        Cut_Ini_Parser_Token *name,
        Cut_Ini_Parser_State *state,
        Cut_Ini_Section *section,
        Cut_Parse_Ini_Result *result
    )
    {
        Cut_Ini_Parser_Token assignment_op_tok = cut_ini_parser_get_next_token(state);
        if (assignment_op_tok.kind != cut_ini_parser_token_kind_op) {
            result->error.msg = cut_fstr0("Assignment operator expected after entry declaration.");
            result->error.line = state->line;
            return;
        }

        Cut_Ini_Parser_Token value_tok = cut_ini_parser_get_next_token(state);
        if (value_tok.kind != cut_ini_parser_token_kind_name_or_value) {
            result->error.msg = cut_fstr0("Value expected after assignment operation.");
            result->error.line = state->line;
            return;
        }

        Cut_Ini_Field field;
        field.name_start = name->start;
        field.name_end = name->end;
        field.val_start = value_tok.start;
        field.val_end = value_tok.end;

        if (section->fields.count >= section->fields.capacity) {
            cut_u32 new_capacity = section->fields.capacity * 2;
            section->fields.data = cut_mem_resize(
                sizeof(Cut_Ini_Field) * new_capacity,
                section->fields.data,
                sizeof(Cut_Ini_Field) * section->fields.capacity,
                &result->mem_allocator
            );
            section->fields.capacity = new_capacity;
        }
        section->fields.data[section->fields.count] = field;
        section->fields.count++;
    }

    cut_internal void cut_ini_parse_section(
        Cut_Ini_Parser_Token *tok,
        Cut_Ini_Parser_State *state,
        Cut_Parse_Ini_Result *result
    )
    {
        // @TODO: I do not want to check this here. Since this function is called, it should be sure that the separator is indeed '['.
        if (state->data[tok->start] == ']') {
            result->error.msg = cut_fstr0("Section declaration should start with '[' but it was ']'.");
            result->error.line = state->line;
            return;
        }

        Cut_Ini_Parser_Token section_name_tok = cut_ini_parser_get_next_token(state);
        if (section_name_tok.kind != cut_ini_parser_token_kind_name_or_value) {
            result->error.msg = cut_fstr0("A name for the section was expected.");
            result->error.line = state->line;
            return;
        }

        Cut_Ini_Parser_Token section_end_sep_tok = cut_ini_parser_get_next_token(state);
        if (section_end_sep_tok.kind != cut_ini_parser_token_kind_separator) {
            result->error.msg = cut_fstr0("Separator expected at the end of section declaration.");
            result->error.line = state->line;
            return;
        }
        if (state->data[section_end_sep_tok.start] == '[') {
            result->error.msg = cut_fstr0("Section declaration should end with ']' but it was '['.");
            result->error.line = state->line;
            return;
        }

        Cut_Ini_Section section;
        section.decl_start = tok->start;
        section.decl_end = section_end_sep_tok.start;
        section.name = section_name_tok.start;
        section.name_size = section_name_tok.end - section_name_tok.start + 1;

        section.fields.data = cut_mem_allocate(sizeof(Cut_Ini_Field) * CUT_PARSE_INI_SECTION_FIELDS_INIT_CAPACITY, &result->mem_allocator);
        section.fields.count = 0;
        section.fields.capacity = CUT_PARSE_INI_SECTION_FIELDS_INIT_CAPACITY;

        int loop = 1;
        while (loop) {
            // @TODO: Maybe handle multi errors instead of exiting at the first error ?
            if (result->error.msg.data != 0) break;

            Cut_Ini_Parser_Token next_tok = cut_ini_parser_get_next_token(state);

            switch (next_tok.kind) {
                case cut_ini_parser_token_kind_name_or_value:
                    if (section.fields.count == 0) // Represents begin of content if this is the first field.
                        section.content_start = next_tok.start;
                    cut_ini_parse_entry(&next_tok, state, &section, result);
                    break;

                    // '[' has been encountered, let's finish with this section and parse a new one.
                case cut_ini_parser_token_kind_separator:
                    if (section.fields.count == 0) // Begin of content equals end of content if there is no field.
                        section.content_start = next_tok.start - 1;
                    section.content_end = next_tok.start - 1;
                    cut_ini_parse_section(&next_tok, state, result);
                    loop = 0;
                    break;

                case cut_ini_parser_token_kind_end:
                    section.content_end = state->index;
                    if (section.fields.count == 0) // Begin of content equals end of content if there is no field.
                        section.content_start = section.content_end;
                    loop = 0;
                    break;

                default:
                    result->error.msg = cut_fstr0("Encountered an unknown token!");
                    result->error.line = state->line;
                    break;
            }
        }

        if (result->sections.count >= result->sections.capacity) {
            cut_u32 new_capacity = result->sections.capacity * 2;
            result->sections.data = cut_mem_resize(
                sizeof(*result->sections.data) * new_capacity,
                result->sections.data,
                sizeof(*result->sections.data) * result->sections.capacity,
                &result->mem_allocator
            );
            result->sections.capacity = new_capacity;
        }
        result->sections.data[result->sections.count] = section;
        result->sections.count++;
    }

    cut_internal void cut_ini_parse_global_section(
        Cut_Ini_Parser_State *state,
        Cut_Parse_Ini_Result *result
    )
    {
        result->global_section.decl_start = 0;
        result->global_section.decl_end = 0;
        result->global_section.content_start = state->index;
        result->global_section.content_end = state->index;
        result->global_section.name = 0;
        result->global_section.name_size = 0;

        result->global_section.fields.data = cut_mem_allocate(sizeof(Cut_Ini_Field) * CUT_PARSE_INI_SECTION_FIELDS_INIT_CAPACITY, &result->mem_allocator);
        result->global_section.fields.count = 0;
        result->global_section.fields.capacity = CUT_PARSE_INI_SECTION_FIELDS_INIT_CAPACITY;

        int loop = 1;
        while (loop) {
            // @TODO: Handle multi errors maybe instead of exiting at the first error in the data ?
            if (result->error.msg.data != 0) break;

            Cut_Ini_Parser_Token tok = cut_ini_parser_get_next_token(state);

            switch (tok.kind) {
                // '[' has been encountered, let's finish with this section and parse a new one.
                // New section. No more in global section.
                case cut_ini_parser_token_kind_separator:
                    result->global_section.content_end = tok.start - 1;
                    cut_ini_parse_section(&tok, state, result);
                    return;

                case cut_ini_parser_token_kind_name_or_value:
                    cut_ini_parse_entry(&tok, state, &result->global_section, result);
                    break;

                case cut_ini_parser_token_kind_end:
                    result->global_section.content_end = state->index;
                    loop = 0;
                    break;

                default:
                    result->error.msg = cut_fstr0("Encountered an unknown token!");
                    result->error.line = state->line;
                    break;
            }
        }
    }

    Cut_Parse_Ini_Result cut_ini_parse(cut_u8arrview data, Cut_Mem_Allocator mem_allocator)
    {
        Cut_Parse_Ini_Result result;
        result.mem_allocator = mem_allocator;
        result.error.msg.data = 0;
        result.ini_data = data;
        result.sections.data = cut_mem_allocate(
            sizeof(Cut_Ini_Section) * CUT_PARSE_INI_SECTIONS_INIT_CAPACITY,
            &result.mem_allocator
        );
        result.sections.count = 0;
        result.sections.capacity = CUT_PARSE_INI_SECTIONS_INIT_CAPACITY;

        Cut_Ini_Parser_State state;
        state.data = data.data;
        state.data_size = data.count;
        state.index = 0;
        state.line = 1;

        cut_ini_parse_global_section(&state, &result);
        return result;
    }

    void cut_ini_destroy_parsed_data(Cut_Parse_Ini_Result *parsed_data)
    {
        cut_mem_free(parsed_data->global_section.fields.data, &parsed_data->mem_allocator);

        for (cut_u32 i = 0; i < parsed_data->sections.count; i++) {
            cut_mem_free(parsed_data->sections.data[i].fields.data, &parsed_data->mem_allocator);
        }

        cut_mem_free(parsed_data->sections.data, &parsed_data->mem_allocator);
    }

    Cut_Ini_Get_Entry_Result cut_ini_get_section_entry_value(Cut_Parse_Ini_Result *parsed_data, Cut_Ini_Section *section, cut_u8arrview entry_name)
    {
        Cut_Ini_Get_Entry_Result result;
        result.found = 0;

        for (cut_u32 i = 0; i < section->fields.count; i++) {
            Cut_Ini_Field *field = &section->fields.data[i];

            if (field->name_end - field->name_start + 1 != entry_name.count)
                continue;
            if (cut_mem_cmp(entry_name.data, parsed_data->ini_data.data + field->name_start, entry_name.count))
                continue;

            result.found = 1;
            result.value_start = field->val_start;
            result.value_size = field->val_end - field->val_start + 1;

            break;
        }
        return result;
    }

    Cut_Ini_Get_Entry_Result cut_ini_get_global_entry_value(Cut_Parse_Ini_Result *parsed_data, cut_u8arrview entry_name)
    {
        return cut_ini_get_section_entry_value(parsed_data, &parsed_data->global_section, entry_name);
    }

    Cut_Ini_Get_Entry_Result cut_ini_get_entry_value(Cut_Parse_Ini_Result *parsed_data, cut_u8arrview section_name, cut_u8arrview entry_name)
    {
        for (cut_u32 i = 0; i < parsed_data->sections.count; i++) {
            Cut_Ini_Section *section = &parsed_data->sections.data[i];

            if (section->name_size != section_name.count)
                continue;

            if (cut_mem_cmp(section_name.data, parsed_data->ini_data.data + section->name, section->name_size))
                continue;

            // SECTION FOUND!
            return cut_ini_get_section_entry_value(parsed_data, section, entry_name);
        }

        Cut_Ini_Get_Entry_Result result;
        result.found = 0;
        return result;
    }

    int cut_ini_get_field(cut_u8arrview ini_data, cut_u8arrview section_name, cut_u8arrview entry_name, Cut_Ini_Field *out)
    {
        Cut_Ini_Parser_State state = {
            ini_data.data, ini_data.count, 0, 1
        };

        Cut_Ini_Parser_Token tok;
        if (section_name.data != 0) {
            while (1) {
                tok = cut_ini_parser_get_next_token(&state);
                if (tok.kind == cut_ini_parser_token_kind_end || tok.kind == cut_ini_parser_token_kind_unknown) {
                    return 0;
                }

                if (ini_data.data[tok.start] == '[') {
                    tok = cut_ini_parser_get_next_token(&state);
                    if (tok.kind == cut_ini_parser_token_kind_name_or_value) {
                        // Section name length is different than the given one so this is not the good section.
                        if (section_name.count != tok.end - tok.start + 1)
                            continue;

                        if (!cut_mem_cmp(section_name.data, ini_data.data + tok.start, section_name.count))
                            break;
                    }
                }
            }
        }

        while (1) {
            tok = cut_ini_parser_get_next_token(&state);
            if (tok.kind == cut_ini_parser_token_kind_end || tok.kind == cut_ini_parser_token_kind_unknown) {
                return 0;
            }

            if (tok.kind == cut_ini_parser_token_kind_name_or_value) {
                if (entry_name.count != tok.end - tok.start + 1)
                    continue; // Entry name size does not match so we already know it's not the good one.

                if (!cut_mem_cmp(entry_name.data, ini_data.data + tok.start, entry_name.count)) {
                    out->name_start = tok.start;
                    out->name_end = tok.end;
                    tok = cut_ini_parser_get_next_token(&state);
                    if (tok.kind == cut_ini_parser_token_kind_op && ini_data.data[tok.start] == '=') {
                        tok = cut_ini_parser_get_next_token(&state);
                        if (tok.kind == cut_ini_parser_token_kind_name_or_value) {
                            out->val_start = tok.start;
                            out->val_end = tok.end;
                            return 1;
                        }
                    }
                    return 0;
                }
            } else if (tok.kind == cut_ini_parser_token_kind_separator) {
                return 0;
            }
        }

        return 0;
    }

    int cut_ini_get_global_field(cut_u8arrview ini_data, cut_u8arrview entry_name, Cut_Ini_Field *out)
    {
        return cut_ini_get_field(ini_data, (cut_u8arrview){0,0}, entry_name, out);
    }
#endif
