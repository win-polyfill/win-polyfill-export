#pragma once

#include <windows.h>
#include <esent.h>

EXTERN_C_START

JET_ERR JET_API
JetAddColumn(
    _In_ JET_SESID                               sesid,
    _In_ JET_TABLEID                             tableid,
    _In_ JET_PCSTR                               szColumnName,
    _In_ const JET_COLUMNDEF *                   pcolumndef,
    _In_reads_bytes_opt_( cbDefault ) JET_PCVOID pvDefault,
    _In_ JET_UINT32                              cbDefault,
    _Out_opt_ JET_COLUMNID *                     pcolumnid );

JET_ERR JET_API
JetAttachDatabase(
    _In_ JET_SESID  sesid,
    _In_ JET_PCSTR  szFilename,
    _In_ JET_GRBIT  grbit );

JET_ERR JET_API
JetAttachDatabase2(
    _In_ JET_SESID              sesid,
    _In_ JET_PCSTR              szFilename,
    _In_ const JET_UINT32       cpgDatabaseSizeMax,
    _In_ JET_GRBIT              grbit );

JET_ERR JET_API
JetBackup(
    _In_ JET_PCSTR      szBackupPath,
    _In_ JET_GRBIT      grbit,
    _In_opt_ JET_PFNSTATUS  pfnStatus );

JET_ERR JET_API
JetBackupInstance(
    _In_ JET_INSTANCE   instance,
    _In_ JET_PCSTR      szBackupPath,
    _In_ JET_GRBIT      grbit,
    _In_opt_ JET_PFNSTATUS  pfnStatus );

JET_ERR JET_API JetBeginSession(
    _In_ JET_INSTANCE   instance,
    _Out_ JET_SESID *   psesid,
    _In_opt_ JET_PCSTR  szUserName,
    _In_opt_ JET_PCSTR  szPassword );

JET_ERR JET_API
JetCompact(
    _In_ JET_SESID              sesid,
    _In_ JET_PCSTR              szDatabaseSrc,
    _In_ JET_PCSTR              szDatabaseDest,
    _In_ JET_PFNSTATUS          pfnStatus,
    _In_opt_ JET_CONVERT_A *    pconvert,
    _In_ JET_GRBIT              grbit );

JET_ERR JET_API
JetCreateDatabase(
    _In_ JET_SESID      sesid,
    _In_ JET_PCSTR      szFilename,
    _In_opt_ JET_PCSTR  szConnect,
    _Out_ JET_DBID *    pdbid,
    _In_ JET_GRBIT      grbit );

JET_ERR JET_API
JetCreateDatabase2(
    _In_ JET_SESID              sesid,
    _In_ JET_PCSTR              szFilename,
    _In_ const JET_UINT32       cpgDatabaseSizeMax,
    _Out_ JET_DBID *            pdbid,
    _In_ JET_GRBIT              grbit );

JET_ERR JET_API
JetCreateIndex(
    _In_ JET_SESID                      sesid,
    _In_ JET_TABLEID                    tableid,
    _In_ JET_PCSTR                      szIndexName,
    _In_ JET_GRBIT                      grbit,
    _In_reads_bytes_( cbKey ) JET_PCSTR szKey,
    _In_ JET_UINT32                     cbKey,
    _In_ JET_UINT32                     lDensity );

JET_ERR JET_API
JetCreateIndex2(
    _In_ JET_SESID                                  sesid,
    _In_ JET_TABLEID                                tableid,
    _In_reads_( cIndexCreate ) JET_INDEXCREATE_A *  pindexcreate,
    _In_ JET_UINT32                                 cIndexCreate );

JET_ERR JET_API
JetCreateInstance(
    _Out_ JET_INSTANCE *    pinstance,
    _In_opt_ JET_PCSTR      szInstanceName );

JET_ERR JET_API
JetCreateInstance2(
    _Out_ JET_INSTANCE *    pinstance,
    _In_opt_ JET_PCSTR      szInstanceName,
    _In_opt_ JET_PCSTR      szDisplayName,
    _In_ JET_GRBIT          grbit );

JET_ERR JET_API
JetCreateTable(
    _In_ JET_SESID       sesid,
    _In_ JET_DBID        dbid,
    _In_ JET_PCSTR       szTableName,
    _In_ JET_UINT32      lPages,
    _In_ JET_UINT32      lDensity,
    _Out_ JET_TABLEID *  ptableid );

JET_ERR JET_API
JetCreateTableColumnIndex(
    _In_ JET_SESID               sesid,
    _In_ JET_DBID                dbid,
    _Inout_ JET_TABLECREATE_A *  ptablecreate );

JET_ERR JET_API
JetCreateTableColumnIndex2(
    _In_ JET_SESID                  sesid,
    _In_ JET_DBID                   dbid,
    _Inout_ JET_TABLECREATE2_A *    ptablecreate );

JET_ERR JET_API
JetDefragment(
    _In_ JET_SESID              sesid,
    _In_ JET_DBID               dbid,
    _In_opt_ JET_PCSTR          szTableName,
    _Inout_opt_ JET_UINT32 *    pcPasses,
    _Inout_opt_ JET_UINT32 *    pcSeconds,
    _In_ JET_GRBIT              grbit );


JET_ERR JET_API
JetDefragment2(
    _In_ JET_SESID              sesid,
    _In_ JET_DBID               dbid,
    _In_opt_ JET_PCSTR          szTableName,
    _Inout_opt_ JET_UINT32 *    pcPasses,
    _Inout_opt_ JET_UINT32 *    pcSeconds,
    _In_ JET_CALLBACK           callback,
    _In_ JET_GRBIT              grbit );

JET_ERR JET_API
JetDeleteColumn(
    _In_ JET_SESID      sesid,
    _In_ JET_TABLEID    tableid,
    _In_ JET_PCSTR      szColumnName );

JET_ERR JET_API
JetDeleteColumn2(
    _In_ JET_SESID          sesid,
    _In_ JET_TABLEID        tableid,
    _In_ JET_PCSTR          szColumnName,
    _In_ const JET_GRBIT    grbit );

JET_ERR JET_API
JetDeleteIndex(
    _In_ JET_SESID      sesid,
    _In_ JET_TABLEID    tableid,
    _In_ JET_PCSTR      szIndexName );

JET_ERR JET_API
JetDeleteTable(
    _In_ JET_SESID  sesid,
    _In_ JET_DBID   dbid,
    _In_ JET_PCSTR  szTableName );

JET_ERR JET_API
JetDetachDatabase(
    _In_ JET_SESID  sesid,
    _In_opt_ JET_PCSTR  szFilename );


JET_ERR JET_API
JetEnableMultiInstance(
    _In_reads_opt_( csetsysparam ) JET_SETSYSPARAM_A *  psetsysparam,
    _In_ JET_UINT32                                     csetsysparam,
    _Out_opt_ JET_UINT32 *                              pcsetsucceed );

JET_ERR JET_API
JetDetachDatabase2(
    _In_ JET_SESID  sesid,
    _In_opt_ JET_PCSTR  szFilename,
    _In_ JET_GRBIT  grbit);

JET_ERR JET_API
JetExternalRestore(
    _In_ JET_PSTR                                   szCheckpointFilePath,
    _In_ JET_PSTR                                   szLogPath,
    _In_reads_opt_( crstfilemap ) JET_RSTMAP_A *    rgrstmap,
    _In_ JET_INT32                                  crstfilemap,
    _In_ JET_PSTR                                   szBackupLogPath,
    _In_ JET_INT32                                  genLow,
    _In_ JET_INT32                                  genHigh,
    _In_ JET_PFNSTATUS                              pfn );

JET_ERR JET_API
JetExternalRestore2(
    _In_ JET_PSTR                                   szCheckpointFilePath,
    _In_ JET_PSTR                                   szLogPath,
    _In_reads_opt_( crstfilemap ) JET_RSTMAP_A *    rgrstmap,
    _In_ JET_INT32                                  crstfilemap,
    _In_ JET_PSTR                                   szBackupLogPath,
    _Inout_ JET_LOGINFO_A *                         pLogInfo,
    _In_opt_ JET_PSTR                               szTargetInstanceName,
    _In_opt_ JET_PSTR                               szTargetInstanceLogPath,
    _In_opt_ JET_PSTR                               szTargetInstanceCheckpointPath,
    _In_ JET_PFNSTATUS                              pfn );

JET_ERR JET_API
JetGetAttachInfo(
    _Out_writes_bytes_to_opt_( cbMax, *pcbActual ) JET_PVOID pv,
    _In_ JET_UINT32                                     cbMax,
    _Out_opt_ JET_UINT32 *                              pcbActual );

JET_ERR JET_API
JetGetAttachInfoInstance(
    _In_ JET_INSTANCE                                        instance,
    _Out_writes_bytes_to_opt_( cbMax, *pcbActual ) JET_PVOID pv,
    _In_ JET_UINT32                                          cbMax,
    _Out_opt_ JET_UINT32 *                                   pcbActual );

JET_ERR JET_API
JetGetColumnInfo(
    _In_ JET_SESID                        sesid,
    _In_ JET_DBID                         dbid,
    _In_ JET_PCSTR                        szTableName,
    _In_opt_ JET_PCSTR                    pColumnNameOrId,
    _Out_writes_bytes_( cbMax ) JET_PVOID pvResult,
    _In_ JET_UINT32                       cbMax,
    _In_ JET_UINT32                       InfoLevel );

JET_ERR JET_API
JetGetCurrentIndex(
    _In_ JET_SESID                          sesid,
    _In_ JET_TABLEID                        tableid,
    _Out_writes_bytes_( cbIndexName ) JET_PSTR  szIndexName,
    _In_ JET_UINT32                         cbIndexName );

JET_ERR JET_API
JetGetDatabaseFileInfo(
    _In_ JET_PCSTR                        szDatabaseName,
    _Out_writes_bytes_( cbMax ) JET_PVOID pvResult,
    _In_ JET_UINT32                       cbMax,
    _In_ JET_UINT32                       InfoLevel );

JET_ERR JET_API JetGetDatabaseInfo(
    _In_ JET_SESID                        sesid,
    _In_ JET_DBID                         dbid,
    _Out_writes_bytes_( cbMax ) JET_PVOID pvResult,
    _In_ JET_UINT32                       cbMax,
    _In_ JET_UINT32                       InfoLevel );

JET_ERR JET_API
JetGetIndexInfo(
    _In_ JET_SESID                           sesid,
    _In_ JET_DBID                            dbid,
    _In_ JET_PCSTR                           szTableName,
    _In_opt_ JET_PCSTR                       szIndexName,
    _Out_writes_bytes_( cbResult ) JET_PVOID pvResult,
    _In_ JET_UINT32                          cbResult,
    _In_ JET_UINT32                          InfoLevel );

JET_ERR JET_API
JetGetInstanceInfo(
    _Out_ JET_UINT32 *                                                pcInstanceInfo,
    _Outptr_result_buffer_( *pcInstanceInfo ) JET_INSTANCE_INFO_A **  paInstanceInfo );

JET_ERR JET_API
JetGetLogInfo(
    _Out_writes_bytes_to_opt_( cbMax, *pcbActual ) JET_PVOID pv,
    _In_ JET_UINT32                                          cbMax,
    _Out_opt_ JET_UINT32 *                                   pcbActual );

JET_ERR JET_API
JetGetLogInfoInstance(
    _In_ JET_INSTANCE                                        instance,
    _Out_writes_bytes_to_opt_( cbMax, *pcbActual ) JET_PVOID pv,
    _In_ JET_UINT32                                          cbMax,
    _Out_opt_ JET_UINT32 *                                   pcbActual );

JET_ERR JET_API
JetGetLogInfoInstance2(
    _In_ JET_INSTANCE                                        instance,
    _Out_writes_bytes_to_opt_( cbMax, *pcbActual ) JET_PVOID pv,
    _In_ JET_UINT32                                          cbMax,
    _Out_opt_ JET_UINT32 *                                   pcbActual,
    _Inout_opt_ JET_LOGINFO_A *                              pLogInfo );

JET_ERR JET_API
JetGetObjectInfo(
    _In_ JET_SESID                        sesid,
    _In_ JET_DBID                         dbid,
    _In_ JET_OBJTYP                       objtyp,
    _In_opt_ JET_PCSTR                    szContainerName,
    _In_opt_ JET_PCSTR                    szObjectName,
    _Out_writes_bytes_( cbMax ) JET_PVOID pvResult,
    _In_ JET_UINT32                       cbMax,
    _In_ JET_UINT32                       InfoLevel );

JET_ERR JET_API
JetGetSystemParameter(
    _In_ JET_INSTANCE                   instance,
    _In_opt_ JET_SESID                  sesid,
    _In_ JET_UINT32                     paramid,
    _Out_opt_ JET_API_PTR *             plParam,
    _Out_writes_bytes_opt_( cbMax ) JET_PSTR    szParam,
    _In_ JET_UINT32                     cbMax );

JET_ERR JET_API
JetGetTableColumnInfo(
    _In_ JET_SESID                        sesid,
    _In_ JET_TABLEID                      tableid,
    _In_opt_ JET_PCSTR                    szColumnName,
    _Out_writes_bytes_( cbMax ) JET_PVOID pvResult,
    _In_ JET_UINT32                       cbMax,
    _In_ JET_UINT32                       InfoLevel );

JET_ERR JET_API
JetGetTableIndexInfo(
    _In_ JET_SESID                           sesid,
    _In_ JET_TABLEID                         tableid,
    _In_opt_ JET_PCSTR                       szIndexName,
    _Out_writes_bytes_( cbResult ) JET_PVOID pvResult,
    _In_ JET_UINT32                          cbResult,
    _In_ JET_UINT32                          InfoLevel );

JET_ERR JET_API
JetGetTableInfo(
    _In_ JET_SESID                        sesid,
    _In_ JET_TABLEID                      tableid,
    _Out_writes_bytes_( cbMax ) JET_PVOID pvResult,
    _In_ JET_UINT32                       cbMax,
    _In_ JET_UINT32                       InfoLevel );

JET_ERR JET_API
JetGetTruncateLogInfoInstance(
    _In_ JET_INSTANCE                                        instance,
    _Out_writes_bytes_to_opt_( cbMax, *pcbActual ) JET_PVOID pv,
    _In_ JET_UINT32                                          cbMax,
    _Out_opt_ JET_UINT32 *                                   pcbActual );

JET_ERR JET_API
JetInit3(
    _Inout_opt_ JET_INSTANCE *  pinstance,
    _In_opt_ JET_RSTINFO_A *    prstInfo,
    _In_ JET_GRBIT              grbit );

JET_ERR JET_API
JetOSSnapshotFreeze(
    _In_ const JET_OSSNAPID                                           snapId,
    _Out_ JET_UINT32 *                                                pcInstanceInfo,
    _Outptr_result_buffer_( *pcInstanceInfo ) JET_INSTANCE_INFO_A **  paInstanceInfo,
    _In_ const JET_GRBIT                                              grbit );

JET_ERR JET_API
JetOpenDatabase(
    _In_ JET_SESID      sesid,
    _In_ JET_PCSTR      szFilename,
    _In_opt_ JET_PCSTR  szConnect,
    _Out_ JET_DBID *    pdbid,
    _In_ JET_GRBIT      grbit );

JET_ERR JET_API
JetOpenFile(
    _In_ JET_PCSTR          szFileName,
    _Out_ JET_HANDLE *      phfFile,
    _Out_ JET_UINT32 *      pulFileSizeLow,
    _Out_ JET_UINT32 *      pulFileSizeHigh );

JET_ERR JET_API
JetOpenFileInstance(
    _In_ JET_INSTANCE       instance,
    _In_ JET_PCSTR          szFileName,
    _Out_ JET_HANDLE *      phfFile,
    _Out_ JET_UINT32 *      pulFileSizeLow,
    _Out_ JET_UINT32 *      pulFileSizeHigh );

JET_ERR JET_API
JetOpenTable(
    _In_ JET_SESID                                  sesid,
    _In_ JET_DBID                                   dbid,
    _In_ JET_PCSTR                                  szTableName,
    _In_reads_bytes_opt_( cbParameters ) JET_PCVOID pvParameters,
    _In_ JET_UINT32                                 cbParameters,
    _In_ JET_GRBIT                                  grbit,
    _Out_ JET_TABLEID *                             ptableid );

JET_ERR JET_API
JetRenameColumn(
    _In_ JET_SESID      sesid,
    _In_ JET_TABLEID    tableid,
    _In_ JET_PCSTR      szName,
    _In_ JET_PCSTR      szNameNew,
    _In_ JET_GRBIT      grbit );

JET_ERR JET_API
JetRenameTable(
    _In_ JET_SESID  sesid,
    _In_ JET_DBID   dbid,
    _In_ JET_PCSTR  szName,
    _In_ JET_PCSTR  szNameNew );

JET_ERR JET_API
JetRestore(
    _In_ JET_PCSTR      szSource,
    _In_opt_ JET_PFNSTATUS  pfn );

JET_ERR JET_API
JetRestore2(
    _In_ JET_PCSTR      sz,
    _In_opt_ JET_PCSTR  szDest,
    _In_opt_ JET_PFNSTATUS  pfn );

JET_ERR JET_API
JetRestoreInstance(
    _In_ JET_INSTANCE   instance,
    _In_ JET_PCSTR      sz,
    _In_opt_ JET_PCSTR  szDest,
    _In_opt_ JET_PFNSTATUS  pfn );

JET_ERR JET_API
JetSetColumnDefaultValue(
    _In_ JET_SESID                        sesid,
    _In_ JET_DBID                         dbid,
    _In_ JET_PCSTR                        szTableName,
    _In_ JET_PCSTR                        szColumnName,
    _In_reads_bytes_( cbData ) JET_PCVOID pvData,
    _In_ const JET_UINT32                 cbData,
    _In_ const JET_GRBIT                  grbit );

JET_ERR JET_API
JetSetCurrentIndex(
    _In_ JET_SESID      sesid,
    _In_ JET_TABLEID    tableid,
    _In_opt_ JET_PCSTR  szIndexName );

JET_ERR JET_API
JetSetCurrentIndex2(
    _In_ JET_SESID      sesid,
    _In_ JET_TABLEID    tableid,
    _In_opt_ JET_PCSTR  szIndexName,
    _In_ JET_GRBIT      grbit );

JET_ERR JET_API
JetSetCurrentIndex3(
    _In_ JET_SESID      sesid,
    _In_ JET_TABLEID    tableid,
    _In_opt_ JET_PCSTR  szIndexName,
    _In_ JET_GRBIT      grbit,
    _In_ JET_UINT32     itagSequence );

JET_ERR JET_API
JetSetCurrentIndex4(
    _In_ JET_SESID          sesid,
    _In_ JET_TABLEID        tableid,
    _In_opt_ JET_PCSTR      szIndexName,
    _In_opt_ JET_INDEXID *  pindexid,
    _In_ JET_GRBIT          grbit,
    _In_ JET_UINT32         itagSequence );

JET_ERR JET_API
JetSetDatabaseSize(
    _In_ JET_SESID          sesid,
    _In_ JET_PCSTR          szDatabaseName,
    _In_ JET_UINT32         cpg,
    _Out_ JET_UINT32 *      pcpgReal );

JET_ERR JET_API
JetSetSystemParameter(
    _Inout_opt_ JET_INSTANCE *  pinstance,
    _In_opt_ JET_SESID          sesid,
    _In_ JET_UINT32             paramid,
    _In_opt_ JET_API_PTR        lParam,
    _In_opt_ JET_PCSTR          szParam );

JET_ERR JET_API
JetDefragment3(
    _In_ JET_SESID              sesid,
    _In_ JET_PCSTR              szDatabaseName,
    _In_opt_ JET_PCSTR          szTableName,
    _Inout_opt_ JET_UINT32 *    pcPasses,
    _Inout_opt_ JET_UINT32 *    pcSeconds,
    _In_ JET_CALLBACK           callback,
    _In_ JET_PVOID              pvContext,
    _In_ JET_GRBIT              grbit );

JET_ERR JET_API
JetOSSnapshotGetFreezeInfo(
    _In_ const JET_OSSNAPID                                           snapId,
    _Out_ JET_UINT32 *                                                pcInstanceInfo,
    _Outptr_result_buffer_( *pcInstanceInfo ) JET_INSTANCE_INFO_A **  paInstanceInfo,
    _In_ const JET_GRBIT                                              grbit );

EXTERN_C_END
