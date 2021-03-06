/*
  NoirVisor - Hardware-Accelerated Hypervisor solution

  Copyright 2018, Zero Tang. All rights reserved.

  This file is the exit handler of Intel VT-x.

  This program is distributed in the hope that it will be useful, but 
  without any warranty (no matter implied warranty or merchantability
  or fitness for a particular purpose, etc.).

  File Location: /vt_core/vt_exit.c
*/

#include <nvdef.h>
#include <nvstatus.h>
#include <nvbdk.h>
#include <vt_intrin.h>
#include <noirhvm.h>
#include <intrin.h>
#include <ia32.h>
#include "vt_vmcs.h"
#include "vt_exit.h"
#include "vt_def.h"
#include "vt_ept.h"

void nvc_vt_dump_vmcs_guest_state()
{
	ulong_ptr v1,v2,v3,v4;
	nv_dprintf("Dumping VMCS Guest State Area!\n");
	noir_vt_vmread(guest_cs_selector,&v1);
	noir_vt_vmread(guest_cs_access_rights,&v2);
	noir_vt_vmread(guest_cs_limit,&v3);
	noir_vt_vmread(guest_cs_base,&v4);
	nv_dprintf("Guest CS Segment - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_ds_selector,&v1);
	noir_vt_vmread(guest_ds_access_rights,&v2);
	noir_vt_vmread(guest_ds_limit,&v3);
	noir_vt_vmread(guest_ds_base,&v4);
	nv_dprintf("Guest DS Segment - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_es_selector,&v1);
	noir_vt_vmread(guest_es_access_rights,&v2);
	noir_vt_vmread(guest_es_limit,&v3);
	noir_vt_vmread(guest_es_base,&v4);
	nv_dprintf("Guest ES Segment - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_fs_selector,&v1);
	noir_vt_vmread(guest_fs_access_rights,&v2);
	noir_vt_vmread(guest_fs_limit,&v3);
	noir_vt_vmread(guest_fs_base,&v4);
	nv_dprintf("Guest FS Segment - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_gs_selector,&v1);
	noir_vt_vmread(guest_gs_access_rights,&v2);
	noir_vt_vmread(guest_gs_limit,&v3);
	noir_vt_vmread(guest_gs_base,&v4);
	nv_dprintf("Guest GS Segment - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_ss_selector,&v1);
	noir_vt_vmread(guest_ss_access_rights,&v2);
	noir_vt_vmread(guest_ss_limit,&v3);
	noir_vt_vmread(guest_ss_base,&v4);
	nv_dprintf("Guest SS Segment - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_tr_selector,&v1);
	noir_vt_vmread(guest_tr_access_rights,&v2);
	noir_vt_vmread(guest_tr_limit,&v3);
	noir_vt_vmread(guest_tr_base,&v4);
	nv_dprintf("Guest Task Register - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_ldtr_selector,&v1);
	noir_vt_vmread(guest_ldtr_access_rights,&v2);
	noir_vt_vmread(guest_ldtr_limit,&v3);
	noir_vt_vmread(guest_ldtr_base,&v4);
	nv_dprintf("Guest LDT Register - Selector: 0x%X\t Access-Rights: 0x%X\t Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_gdtr_limit,&v1);
	noir_vt_vmread(guest_gdtr_base,&v2);
	noir_vt_vmread(guest_idtr_limit,&v3);
	noir_vt_vmread(guest_idtr_base,&v4);
	nv_dprintf("Guest GDTR Limit: 0x%X\t Base: 0x%p\t Guest IDTR Limit: 0x%X\t Base: 0x%p\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_cr0,&v1);
	noir_vt_vmread(guest_cr3,&v2);
	noir_vt_vmread(guest_cr4,&v3);
	noir_vt_vmread(guest_dr7,&v4);
	nv_dprintf("Guest Control & Debug Registers - CR0: 0x%X\t CR3: 0x%p\t CR4: 0x%X\t DR7: 0x%X\n",v1,v2,v3,v4);
	noir_vt_vmread(guest_rsp,&v1);
	noir_vt_vmread(guest_rip,&v2);
	noir_vt_vmread(guest_rflags,&v3);
	nv_dprintf("Guest Special GPRs: Rsp: 0x%p\t Rip: 0x%p\t RFlags:0x%X\n",v1,v2,v3);
	noir_vt_vmread(vmcs_link_pointer,&v4);
	nv_dprintf("Guest VMCS Link Pointer: 0x%p\n",v4);
}

//This is the default exit-handler for unexpected VM-Exit.
//You might want to debug your code if this function is invoked.
void static fastcall nvc_vt_default_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_dprintf("Unhandled VM-Exit!, Exit Reason: %s\n",vmx_exit_msg[exit_reason]);
}

//Expected Exit Reason: 2
//If this handler is invoked, we should crash the system.
//This is VM-Exit of obligation.
void static fastcall nvc_vt_trifault_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_panicf("Triple fault occured! System is crashed!\n");
}

//Expected Exit Reason: 9
//This is not an expected VM-Exit. In handler, we should help to switch task.
//This is VM-Exit of obligation.
void static fastcall nvc_vt_task_switch_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_dprintf("Task switch is intercepted!");
}

//Expected Exit Reason: 10
//This is VM-Exit of obligation.
void static fastcall nvc_vt_cpuid_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	u32 cur_proc=noir_get_current_processor();
	noir_vt_vcpu_p vcpu=&hvm_p->virtual_cpu[cur_proc];
	u32 leaf=(u32)gpr_state->rax;
	u32 subleaf=(u32)gpr_state->rcx;
	if(noir_bt(&leaf,31))
	{
		//At this moment, Extended CPUID leaf is invoked.
		if(leaf>vcpu->relative_hvm->ext_leaftotal+0x80000000)
		{
			//Leaf is invalid. No need to check the cache.
			*(u32*)&gpr_state->rax=0;
			*(u32*)&gpr_state->rbx=0;
			*(u32*)&gpr_state->rcx=0;
			*(u32*)&gpr_state->rdx=0;
		}
		else
		{
			//On Intel Processors, no subleaf exists on extended leaf.
			*(u32*)&gpr_state->rax=vcpu->cpuid_cache.ext_leaf[leaf-0x80000000].eax;
			*(u32*)&gpr_state->rbx=vcpu->cpuid_cache.ext_leaf[leaf-0x80000000].ebx;
			*(u32*)&gpr_state->rcx=vcpu->cpuid_cache.ext_leaf[leaf-0x80000000].ecx;
			*(u32*)&gpr_state->rdx=vcpu->cpuid_cache.ext_leaf[leaf-0x80000000].edx;
		}
	}
	else
	{
		//At this moment, Standard CPUID leaf is invoked.
		if(leaf>vcpu->relative_hvm->std_leaftotal)
		{
			//Leaf is invalid. No need to check the cache.
			*(u32*)&gpr_state->rax=0;
			*(u32*)&gpr_state->rbx=0;
			*(u32*)&gpr_state->rcx=0;
			*(u32*)&gpr_state->rdx=0;
		}
		else
		{
			//We need to check if subleaf hits the cache.
			bool cache_hit=false;
			if(leaf<32)
			{
				if(noir_bt(&vcpu->relative_hvm->cpuid_submask,leaf))
					cache_hit=(subleaf==0);
				else
					cache_hit=true;
			}
			if(cache_hit)
			{
				//If cache is hit, use cached info.
				*(u32*)&gpr_state->rax=vcpu->cpuid_cache.std_leaf[leaf].eax;
				*(u32*)&gpr_state->rbx=vcpu->cpuid_cache.std_leaf[leaf].ebx;
				*(u32*)&gpr_state->rcx=vcpu->cpuid_cache.std_leaf[leaf].ecx;
				*(u32*)&gpr_state->rdx=vcpu->cpuid_cache.std_leaf[leaf].edx;
			}
			else
			{
				//Otherwise, invoke cpuid.
				noir_cpuid(leaf,subleaf,(u32*)&gpr_state->rax,(u32*)&gpr_state->rbx,(u32*)&gpr_state->rcx,(u32*)&gpr_state->rdx);
			}
		}
	}
	noir_vt_advance_rip();
}

//Expected Exit Reason: 11
//We should virtualize the SMX (Safer Mode Extension) in this handler.
//This is VM-Exit of obligation.
void static fastcall nvc_vt_getsec_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_dprintf("SMX Virtualization is not supported!");
	noir_vt_advance_rip();
}

//Expected Exit Reason: 13
//This is VM-Exit of obligation.
void static fastcall nvc_vt_invd_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	//In Hyper-V, it invoked wbinvd at invd exit.
	noir_wbinvd();
	noir_vt_advance_rip();
}

//Expected Exit Reason: 18
//This is VM-Exit of obligation.
void static fastcall nvc_vt_vmcall_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	ulong_ptr gip;
	noir_vt_vmread(guest_rip,&gip);
	//We should verify that the vmcall is inside the NoirVisor's image.
	if(gip>=hvm_p->hv_image.base && gip<hvm_p->hv_image.base+hvm_p->hv_image.size)
	{
		u32 index=(u32)gpr_state->rcx;
		switch(index)
		{
			case noir_vt_callexit:
			{
				noir_vt_vcpu_p vcpu=(noir_vt_vcpu_p)gpr_state->rdx;
				noir_gpr_state_p saved_state=(noir_gpr_state_p)vcpu->hv_stack;
				invept_descriptor ied;
				invvpid_descriptor ivd;
				ulong_ptr gsp,gflags,gcr3;
				u32 inslen=3;		//By default, vmcall uses 3 bytes.
				noir_vt_vmread(guest_rsp,&gsp);
				noir_vt_vmread(guest_rflags,&gflags);
				noir_vt_vmread(guest_cr3,&gcr3);
				noir_vt_vmread(vmexit_instruction_length,&inslen);
				//We may allocate space from unused HV-Stack
				noir_movsp(saved_state,gpr_state,sizeof(void*)*2);
				saved_state->rax=gip+inslen;
				saved_state->rcx=gflags&0x3f7f96;		//Clear ZF and CF bits.
				saved_state->rdx=gsp;
				//Invalidate all TLBs associated with EPT and VPID.
				ied.eptp=0;
				ied.reserved=0;
				noir_vt_invept(ept_global_invd,&ied);
				ivd.vpid=0;
				ivd.reserved[0]=ivd.reserved[1]=ivd.reserved[2]=0;
				ivd.linear_address=0;
				noir_vt_invvpid(vpid_global_invd,&ivd);
				noir_writecr3(gcr3);
				//Mark vCPU is in transition state
				vcpu->status=noir_virt_trans;
				nvc_vt_resume_without_entry(saved_state);
				break;
			}
			default:
			{
				nv_dprintf("Unknown vmcall index!\n");
				break;
			}
		}
	}
	else
	{
		//If vmcall is not inside the NoirVisor's image,
		//we consider this is a regular VMX instruction
		//execution not in VMX Non-Root Operation.
		//Note that this is Nested VMX scenario.
		//Check status of vCPU.
		u32 proc_id=noir_get_current_processor();
		noir_vt_vcpu_p vcpu=&hvm_p->virtual_cpu[proc_id];
		if(vcpu->status==noir_virt_nesting)
		{
			ulong_ptr gflags;
			u64 linked_vmcs;
			noir_vt_vmread(guest_rflags,&gflags);
			noir_vt_vmread(vmcs_link_pointer,&linked_vmcs);
			gflags&=0x3f7702;		//Clear CF PF AF ZF SF OF bits
			if(linked_vmcs==0xffffffffffffffff)
				noir_bts((u32*)&gflags,0);		//At this moment, valid VMCS is not loaded.
			else
				noir_bts((u32*)&gflags,6);		//At this moment, valid VMCS is loaded.
			noir_vt_vmwrite(guest_rflags,gflags);
			noir_vt_advance_rip();
		}
		else
		{
			//At this moment, vCPU did not enter VMX operation.
			//Issue a #UD exception to Guest.
			noir_vt_inject_event(ia32_invalid_opcode,ia32_hardware_exception,false,0);
		}
	}
}

//Expected Exit Reason: 19-27,50,53
//This is VM-Exit of obligation.
void static fastcall nvc_vt_vmx_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_dprintf("VMX Nesting is not implemented! %s\n",vmx_exit_msg[exit_reason]);
	noir_vt_advance_rip();
}

//Expected Exit Reason: 28
//Filter unwanted behaviors.
//Besides, we need to virtualize CR4.VMXE and CR4.SMXE here.
void static fastcall nvc_vt_cr_access_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	//On default NoirVisor's setting of VMCS, this handler only traps writes to CR0 and CR4.
	u32 cur_proc=noir_get_current_processor();
	noir_vt_vcpu_p vcpu=&hvm_p->virtual_cpu[cur_proc];
	ia32_cr_access_qualification info;
	bool advance_ip=true;
	noir_vt_vmread(vmexit_qualification,&info);
	switch(info.access_type)
	{
		case 0:
		{
			//Write to Control Register is intercepted!
			switch(info.cr_num)
			{
				case 0:
				{
					ulong_ptr data=((ulong_ptr*)gpr_state)[info.gpr_num];
					//If Guest attempts to clear bits unsupported by VMX, inject #GP.
					if((data & 0x80000021)!=0x80000021)
					{
						noir_vt_inject_event(ia32_general_protection,ia32_hardware_exception,true,0);
						advance_ip=false;
					}
					//If Guest attempts to set bits unsupported by CPU, inject #GP.
					else if(data & 0x1ffaffc0)
					{
						noir_vt_inject_event(ia32_general_protection,ia32_hardware_exception,true,0);
						advance_ip=false;
					}
					//Otherwise, perform setup.
					else
					{
						noir_vt_vmwrite(guest_cr0,data);
						noir_vt_vmwrite(cr0_read_shadow,data);
					}
					break;
				}
				case 4:
				{
					ulong_ptr data=((ulong_ptr*)gpr_state)[info.gpr_num];
					//If Guest attempts to write CR4.VMXE, mark vCPU as Nexted-VMX-Enabled(Disabled).
					u32 vmxe_mask=noir_bt((u32*)&data,ia32_cr4_vmxe)<<noir_nvt_vmxe;
					vcpu->nested_vcpu.status|=vmxe_mask;
					vcpu->nested_vcpu.status&=~vmxe_mask;
					noir_vt_vmwrite(cr4_read_shadow,vmxe_mask);
					break;
				}
				default:
				{
					nv_dprintf("Unexpected write to Control Register is intercepted!\n");
					break;
				}
			}
			break;
		}
		case 1:
		{
			//Read from Control Register is intercepted!
			nv_dprintf("Unexpected read from Control Register is intercepted!\n");
			break;
		}
		case 2:
		{
			//Clts instruction is intercepted!
			//By default setting of VMCS, we won't go here.
			nv_dprintf("Check your CR0 Guest-Host Mask!\n");
			break;
		}
		case 3:
		{
			//Lmsw instruction is intercepted!
			//We monitor bit 0 of CR0, we might go here as lmsw overwrites bits 0-3.
			ulong_ptr msw=info.lmsw_src&0xf;
			if(noir_bt((u32*)&msw,0))
			{
				ulong_ptr gcr0;
				noir_vt_vmread(guest_cr0,&gcr0);
				gcr0|=msw;
				gcr0&=~msw;
				noir_vt_vmwrite(guest_cr0,gcr0);
			}
			break;
		}
	}
	if(advance_ip)noir_vt_advance_rip();
}

//Expected Exit Reason: 31
//This is the key feature of MSR-Hook Hiding.
void static fastcall nvc_vt_rdmsr_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	u32 index=(u32)gpr_state->rcx;
	large_integer val;
	switch(index)
	{
#if defined(_amd64)
		case ia32_lstar:
		{
			val.value=orig_system_call;
			break;
		}
#else
		case ia32_sysenter_eip:
		{
			val.value=orig_system_call;
			break;
		}
#endif
		default:
		{
			nv_dprintf("Unexpected rdmsr is intercepted! Index=0x%X\n",index);
			val.value=noir_rdmsr(index);
			break;
		}
	}
	*(u32*)&gpr_state->rax=val.low;
	*(u32*)&gpr_state->rdx=val.high;
	noir_vt_advance_rip();
}

//Expected Exit Reason: 33
//This is VM-Exit of obligation.
//You may want to debug your code if this handler is invoked.
void static fastcall nvc_vt_invalid_guest_state(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_dprintf("VM-Entry failed! Check the Guest-State in VMCS!\n");
	nvc_vt_dump_vmcs_guest_state();
}

//Expected Exit Reason: 34
//This is VM-Exit of obligation.
//You may want to debug your code if this handler is invoked.
void static fastcall nvc_vt_invalid_msr_loading(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_dprintf("VM-Entry failed! Check the MSR-Auto list in VMCS!\n");
}

//Expected Exit Reason: 48
//This is VM-Exit of obligation on EPT.
//Specifically, this handler is invoked on EPT-based stealth hook.
void static fastcall nvc_vt_ept_violation_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	noir_hook_page_p nhp=noir_hook_pages;
	u64 gpa;
	noir_vt_vmread(guest_physical_address,&gpa);
	nv_dprintf("EPT Violation Occured at GPA=0x%llX!\n",gpa);
	while(nhp)
	{
		if(gpa>=nhp->orig.phys && gpa<nhp->orig.phys+page_size)
		{
			ia32_ept_violation_qualification info;
			invept_descriptor ied;
			//The violated page is found. Perform page-substitution
			ia32_ept_pte_p pte_p=(ia32_ept_pte_p)nhp->pte_descriptor;
			u32 proc_num=noir_get_current_processor();
			noir_vt_vcpu_p vcpu=&hvm_p->virtual_cpu[proc_num];
			noir_ept_manager_p eptm=(noir_ept_manager_p)vcpu->ept_manager;
			noir_vt_vmread(vmexit_qualification,&info);
			if(info.read || info.write)
			{
				//If the access is read or write, we grant
				//read/write permission but revoke execute permission
				//and substitute the page to be original page
				pte_p->read=1;
				pte_p->write=1;
				pte_p->execute=0;
				pte_p->page_offset=nhp->orig.phys>>12;
			}
			else if(info.execute)
			{
				//If the access is execute, we grant
				//execute permission but revoke read/write permission
				//and substitute the page to be hooked page
				pte_p->read=0;
				pte_p->write=0;
				pte_p->execute=1;
				pte_p->page_offset=nhp->hook.phys>>12;
			}
			//Since Paging Structure is revised, invalidate the EPT TLB.
			ied.reserved=0;
			ied.eptp=eptm->eptp.phys.value;
			noir_vt_invept(ept_single_invd,&ied);
			break;
		}
	}
}

//Expected Exit Reason: 49
//This is VM-Exit of obligation on EPT.
//You may want to debug your code if this handler is invoked.
void static fastcall nvc_vt_ept_misconfig_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	nv_dprintf("EPT Misconfiguration Occured!\n");
	noir_int3();
}

//Expected Exit Reason: 55
//This is VM-Exit of obligation.
void static fastcall nvc_vt_xsetbv_handler(noir_gpr_state_p gpr_state,u32 exit_reason)
{
	//Simply call xsetbv again.
	noir_vt_advance_rip();
}

//It is important that this function uses fastcall convention.
void fastcall nvc_vt_exit_handler(noir_gpr_state_p gpr_state)
{
	u32 exit_reason;
	noir_vt_vmread(vmexit_reason,&exit_reason);
	if((exit_reason&0xFFFF)<vmx_maximum_exit_reason)
		vt_exit_handlers[exit_reason&0xFFFF](gpr_state,exit_reason);
	else
		nvc_vt_default_handler(gpr_state,exit_reason);
	//Guest RIP is supposed to be advanced in specific handlers, not here.
	//Do not execute vmresume here. It will be done as this function returns.
}

noir_status nvc_vt_build_exit_handlers()
{
	vt_exit_handlers=noir_alloc_nonpg_memory(vmx_maximum_exit_reason*sizeof(void*));
	if(vt_exit_handlers)
	{
		//Initialize handler-array with default handler.
		//Use stos to accelerate the initialization and reduce codes.
		noir_stosp(vt_exit_handlers,(ulong_ptr)nvc_vt_default_handler,vmx_maximum_exit_reason);
		vt_exit_handlers[triple_fault]=nvc_vt_trifault_handler;
		vt_exit_handlers[task_switch]=nvc_vt_task_switch_handler;
		vt_exit_handlers[intercept_cpuid]=nvc_vt_cpuid_handler;
		vt_exit_handlers[intercept_getsec]=nvc_vt_getsec_handler;
		vt_exit_handlers[intercept_invd]=nvc_vt_invd_handler;
		vt_exit_handlers[intercept_vmcall]=nvc_vt_vmcall_handler;
		vt_exit_handlers[intercept_vmclear]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmlaunch]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmptrld]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmptrst]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmread]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmresume]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmwrite]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmxoff]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_vmxon]=nvc_vt_vmx_handler;
		vt_exit_handlers[cr_access]=nvc_vt_cr_access_handler;
		vt_exit_handlers[intercept_rdmsr]=nvc_vt_rdmsr_handler;
		vt_exit_handlers[invalid_guest_state]=nvc_vt_invalid_guest_state;
		vt_exit_handlers[msr_loading_failure]=nvc_vt_invalid_msr_loading;
		vt_exit_handlers[ept_violation]=nvc_vt_ept_violation_handler;
		vt_exit_handlers[ept_misconfiguration]=nvc_vt_ept_misconfig_handler;
		vt_exit_handlers[intercept_invept]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_invvpid]=nvc_vt_vmx_handler;
		vt_exit_handlers[intercept_xsetbv]=nvc_vt_xsetbv_handler;
		return noir_success;
	}
	return noir_insufficient_resources;
}

void nvc_vt_teardown_exit_handlers()
{
	if(vt_exit_handlers)
		noir_free_nonpg_memory(vt_exit_handlers);
	vt_exit_handlers=null;
}