/** Every way an archive, a name or an input to the writer can be refused. */
export type MpqErrorCode =
  // the container
  | 'too_small'
  | 'too_large'
  | 'bad_magic'
  | 'bad_header_size'
  | 'unsupported_version'
  | 'bad_sector_size'
  | 'bad_archive_size'
  | 'bad_hash_table'
  | 'bad_block_table'
  | 'bad_hash_entry'
  | 'deleted_entry'
  | 'bad_locale'
  | 'duplicate_block_ref'
  | 'unreferenced_block'
  | 'overlap'
  | 'unreferenced_data'
  // a file's block
  | 'encrypted_file'
  | 'unsupported_compression'
  | 'single_unit'
  | 'unsupported_flags'
  | 'bad_block'
  | 'bad_sector_table'
  | 'bad_zlib'
  | 'file_too_large'
  | 'too_many_files'
  | 'total_too_large'
  | 'ratio_too_high'
  // the listfile and the names in it
  | 'missing_listfile'
  | 'bad_listfile'
  | 'listed_name_missing'
  | 'unlisted_files'
  | 'name_hash_collision'
  | 'bad_name'
  | 'name_too_long'
  | 'path_traversal'
  | 'absolute_path'
  | 'reserved_name'
  | 'special_file'
  | 'duplicate_name';

export interface MpqIssue {
  code: MpqErrorCode;
  message: string;
  /** The file the issue is about, when it is about one. */
  name?: string;
}

export class MpqError extends Error {
  readonly code: MpqErrorCode;
  readonly file?: string;

  constructor(code: MpqErrorCode, message: string, file?: string) {
    super(message);
    this.name = 'MpqError';
    this.code = code;
    if (file !== undefined) this.file = file;
  }

  issue(): MpqIssue {
    return { code: this.code, message: this.message, ...(this.file !== undefined ? { name: this.file } : {}) };
  }
}
