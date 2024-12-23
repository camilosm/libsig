
/*--------------------------------------------------------------------*/
/*--- libsig: a library signature generator.             ls_main.c ---*/
/*--------------------------------------------------------------------*/

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_options.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_machine.h"
#include "pub_tool_addrinfo.h"

VgFile* ls_functions_file;

static IRSB* ls_instrument (VgCallbackClosure* closure,
                      IRSB* bb,
                      const VexGuestLayout* layout,
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy ) {

   IRSB* new_bb = deepCopyIRSBExceptStmts(bb);

   for (Int i=0; i<bb->stmts_used; i++) {
      IRStmt* stmt = bb->stmts[i];
      if (!stmt)
         continue;

      if (stmt->tag == Ist_IMark) {
         Addr addr = stmt->Ist.IMark.addr;
         DiEpoch ep = VG_(current_DiEpoch)();
         const HChar *fn_name;
         Bool found_fn_name;

         found_fn_name = VG_(get_fnname)(ep, addr, &fn_name);
         if (found_fn_name) {
            VG_(fprintf)(ls_functions_file, "0x%lx:%s\n", addr, fn_name);
         } else {
            VG_(fprintf)(ls_functions_file, "0x%lx:unknown\n", addr);
         }
      }

      addStmtToIRSB(new_bb, stmt);
   }

   return new_bb;
}

static void ls_dump_mappings() {
	const HChar* filename = "libsig-map.out";
   VgFile* outfile;
	const DebugInfo* di;

	outfile = VG_(fopen)(filename, VKI_O_WRONLY|VKI_O_TRUNC, 0);
	if (outfile == 0) {
		outfile = VG_(fopen)(filename, VKI_O_CREAT|VKI_O_WRONLY, VKI_S_IRUSR|VKI_S_IWUSR);
	}

	for (di = VG_(next_DebugInfo)(0); di; di = VG_(next_DebugInfo)(di)) {
		Addr addr;
		SizeT size;

		addr = VG_(DebugInfo_get_text_avma)(di);
		if (!addr)
			continue;

		size = VG_(DebugInfo_get_text_size)(di);

		VG_(fprintf)(outfile, "%s:0x%lx:%lu\n", VG_(DebugInfo_get_filename)(di), addr, size);
	}

	VG_(fclose)(outfile);
}

static void ls_post_clo_init(void) {
   if (!VG_(clo_track_fds))
     VG_(needs_core_errors)(False);
}

static void ls_fini(Int exitcode) {
   VG_(fclose)(ls_functions_file);
   ls_dump_mappings();
}

static void ls_pre_clo_init(void) {
   VG_(details_name)            ("libsig");
   VG_(details_version)         ("0.1");
   VG_(details_description)     ("a library signature generator");
   VG_(details_copyright_author)("Copyright (C) 2024, and GNU GPL'd, by Camilo Santana Melgaço.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs) (ls_post_clo_init, ls_instrument, ls_fini);

   const HChar* ls_functions_filename = "libsig-functions.out";
   ls_functions_file = VG_(fopen)(ls_functions_filename, VKI_O_WRONLY|VKI_O_TRUNC, 0);
	if (ls_functions_file == 0) {
		ls_functions_file = VG_(fopen)(ls_functions_filename, VKI_O_CREAT|VKI_O_WRONLY, VKI_S_IRUSR|VKI_S_IWUSR);
	}
}

VG_DETERMINE_INTERFACE_VERSION(ls_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
