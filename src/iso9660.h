#ifndef ISO9660_H
#define ISO9660_H

#include <stdint.h>

#define ISO_BLOCK_SIZE 2048
#define ISO_MAX_DIR_DEPTH 8
#define ISO_STANDARD_ID "CD001"
#define ISO_STANDARD_ID_LEN 5

struct int16_lsb_msb
{
    uint16_t le;
    uint16_t be;
} __attribute__((packed));

struct int32_lsb_msb
{
    uint32_t le;
    uint32_t be;
} __attribute__((packed));

// See: 9.4 - Format of a Path Table Record
struct iso_path_table_le
{
    uint8_t idf_len; // Identifier name length
    uint8_t ext_size; // Extended attribute record length
    uint32_t data_blk; // File data block index
    uint16_t parent_dir; // Number of the parent dir
} __attribute__((packed));

enum iso_file_type
{
    ISO_FILE_HIDDEN = 1, // File is Hidden
    ISO_FILE_ISDIR = 2, // Entry is a Directory
    ISO_FILE_ASSOCIAT = 4, // Entry is an Associated
    ISO_FILE_USEEXT = 8,
    // Information is structured according to the extended attribute record
    ISO_FILE_USEPERM = 16,
    // Permissions are specified in the extended attribute record
    ISO_FILE_MULTIDIR = 128 // File has more than one directory record
};

// See: 9.1 - Format of a Directory Record
#define ISO_DATE_LEN 7

struct iso_dir
{
    uint8_t length; // Length of Directory Record
    uint8_t ext_attr_length; // Extended Attribute Record Length
    struct int32_lsb_msb extent; // Location of Extent
    struct int32_lsb_msb size; // Data Length
    uint8_t date[ISO_DATE_LEN]; // Recording Date and Time
    uint8_t flags; // File Flags
    uint8_t file_unit_size; // File Unit Size
    uint8_t interleave_gap_size; // Interleave Gap Size
    struct int16_lsb_msb volume_sequence_number; // Volume Sequence Number
    uint8_t name_len; // Length of File Identifier
    char name[1]; // File Identifier (variable length)
} __attribute__((packed));

// See: 8.4 - Primary Volume Descriptor
#define ISO_PRIM_VOLDESC_BLOCK 16

#define ISO_SYSIDF_LEN 32
#define ISO_VOLIDF_LEN 32
#define ISO_VOLSET_LEN 128
#define ISO_PUBIDF_LEN 128
#define ISO_DPREP_LEN 128
#define ISO_APP_LEN 128
#define ISO_CPRFIL_LEN 37
#define ISO_ABSFIL_LEN 37
#define ISO_BIBFIL_LEN 37
#define ISO_LDATE_LEN 17

struct iso_prim_voldesc
{
    uint8_t vol_desc_type; // Volume Descriptor Type (1)
    char std_identifier[5]; // Standard Identifier (CD001)
    uint8_t vol_desc_version; // Volume Descriptor Version (1)

    uint8_t unused1[1]; // Unused Field (1 byte)

    char system_identifier[ISO_SYSIDF_LEN]; // System Identifier
    char volume_identifier[ISO_VOLIDF_LEN]; // Volume Identifier

    uint8_t unused2[8]; // Unused Field (8 bytes)

    struct int32_lsb_msb volume_space_size; // Volume Space Size

    uint8_t unused3[32]; // Unused Field (32 bytes)

    struct int16_lsb_msb vol_set_size; // Volume Set Size
    struct int16_lsb_msb vol_seq_number; // Volume Sequence Number
    struct int16_lsb_msb logical_block_size; // Logical Block Size
    struct int32_lsb_msb path_table_size; // Path Table Size

    uint32_t type_l_path_table_loc; // Location of Occurrence of Type L Path Table
    uint32_t opt_type_l_path_table_loc; // Location of Optional Occurrence of Type L Path Table
    uint32_t type_m_path_table_loc; // Location of Occurrence of Type M Path Table
    uint32_t opt_type_m_path_table_loc; // Location of Optional Occurrence of Type M Path Table

    struct iso_dir root_dir; // Root directory entry

    char volume_set_id[ISO_VOLSET_LEN]; // Volume Set Identifier
    char publisher_id[ISO_PUBIDF_LEN]; // Publisher Identifier
    char data_preparer_id[ISO_DPREP_LEN]; // Data Preparer Identifier
    char application_id[ISO_APP_LEN]; // Application Identifier
    char copyright_file_id[ISO_CPRFIL_LEN]; // Copyright File Identifier
    char abstract_file_id[ISO_ABSFIL_LEN]; // Abstract File Identifier
    char bibliographic_file_id[ISO_BIBFIL_LEN]; // Bibliographic File Identifier

    char creation_date[ISO_LDATE_LEN]; // Volume Creation Date and Time
    char modification_date[ISO_LDATE_LEN]; // Volume Modification Date and Time
    char expiration_date[ISO_LDATE_LEN]; // Volume Expiration Date and Time
    char effective_date[ISO_LDATE_LEN]; // Volume Effective Date and Time

    uint8_t file_struct_version; // File Structure Version
    uint8_t unused4[1]; // Unused Field (1 byte)

    uint8_t application_use[512]; // Application Use
    uint8_t reserved[653]; // Reserved for Future Standardization
} __attribute__((packed));

#endif // ISO9660_H
