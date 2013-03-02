/* ============================================================
 * ��Ȩ��    king ��Ȩ���� (c) 2006
 * �ļ���    org.king.check.web.action.DepartmentAction.java
 * �������ڣ� 2006-6-16 10:55:44
 * ���ܣ�    {����Ҫʵ�ֵĹ���}
 * ������:   {��������}
 * �޸ļ�¼��
 * ����                    ����         ����
 * =============================================================
 * 2006-6-16 10:55:44      ljf        �����ļ���ʵ�ֻ�������
 * ============================================================
 */

/**
 * 
 */
package org.king.check.web.action;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;
import org.apache.struts.action.ActionMessages;
import org.apache.struts.action.DynaActionForm;

import org.king.check.config.ClerkConfig;
import org.king.check.domain.Department;
import org.king.check.domain.TCheckWorkconfinfo;
import org.king.check.domain.TCheckWorkconfinfoId;
import org.king.check.service.DepartConfService;
import org.king.check.service.DepartmentService;
import org.king.check.service.SysService;
import org.king.check.service.WorkConfService;
import org.king.check.service.WorkTimeConfService;
import org.king.check.util.DateUtilExtend;
import org.king.check.util.StringUtil;
import org.king.framework.exception.BusinessException;
import org.king.framework.web.action.BaseAction;
import org.springframework.web.util.WebUtils;

public class DepartmentAction extends BaseAction {
	private static final Log log = LogFactory.getLog(DepartmentAction.class);

	private DepartmentService departmentService;

	private WorkTimeConfService workTimeConfService;

	private WorkConfService workConfService;

	private DepartConfService departConfService;

	private SysService sysService;
	public void setSysService(SysService sysService) {
		this.sysService = sysService;
	}
	public void setWorkConfService(WorkConfService workConfService) {
		this.workConfService = workConfService;
	}

	public void setWorkTimeConfService(WorkTimeConfService workTimeConfService) {
		this.workTimeConfService = workTimeConfService;
	}

	public void setDepartmentService(DepartmentService departmentService) {
		this.departmentService = departmentService;
	}

	public void setDepartConfService(DepartConfService departConfService) {
		this.departConfService = departConfService;
	}

	public ActionForward load4add(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		DynaActionForm departmentForm = (DynaActionForm) form;
		Department dept = new Department();
		String pid = (String) request.getParameter("pid");
		if (pid != null) {
			Department entity = departmentService.getDepartment(pid);
			dept.setParent(entity);
		}

		departmentForm.set("department", dept);
		departmentForm.set("methodToCall", "add");

		saveToken(request);

		return (mapping.findForward("input"));
	}

	/**
	 * 
	 * ��������
	 * 
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 */
	public ActionForward create(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		ActionMessages errors = new ActionMessages();
		// Was this transaction cancelled?
		if (isCancelled(request)) {
			removeAttribute(mapping, request);

			return (mapping.findForward("cancel"));
		}

		// check token
		if (!isTokenValid(request)) {
			errors.add(ActionMessages.GLOBAL_MESSAGE, new ActionMessage(
					"errors.TokenError"));
			resetToken(request);
			saveErrors(request, errors);

			return (mapping.getInputForward());
		}

		Department department = (Department) ((DynaActionForm) form)
				.get("department");
		department.setId(request.getParameter("department.id").toString());

		try {
			departmentService.saveDepartment(department);
		} catch (BusinessException be) {
			errors.add("create error", new ActionMessage("errors.UnKnowError"));
		}

		if (!errors.isEmpty()) {
			saveErrors(request, errors);
			saveToken(request);

			return (mapping.getInputForward());
		} else {
			// Remove the obsolete form bean
			removeAttribute(mapping, request);

			return (mapping.findForward("success"));
		}
	}

	/**
	 * װ��Ϊ���޸�
	 * 
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 */
	public final ActionForward load4edit(ActionMapping mapping,
			ActionForm form, HttpServletRequest request,
			HttpServletResponse response) throws Exception {

		DynaActionForm departmentForm = (DynaActionForm) form;
		String deptId = (String) request.getParameter("id");
		Department dept = null;
		if (deptId != null) {
			dept = departmentService.getDepartment(deptId);
		}

		departmentForm.set("department", dept);
		departmentForm.set("methodToCall", "edit");

		saveToken(request);
		return (mapping.findForward("edit"));
	}

	public final ActionForward update(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		ActionMessages errors = new ActionMessages();
		Department department = (Department) ((DynaActionForm) form)
				.get("department");

		try {
			departmentService.updateDepartment(department);
		} catch (BusinessException e) {
			errors.add("update error", new ActionMessage("errors.UnKnowError"));
		}

		if (!errors.isEmpty()) {
			saveErrors(request, errors);
			saveToken(request);

			return (mapping.getInputForward());
		} else {
			// Remove the obsolete form bean
			removeAttribute(mapping, request);

			return (mapping.findForward("success"));
		}
	}

	/**
	 * ɾ��Ŀ¼
	 * 
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 */
	public final ActionForward remove(final ActionMapping mapping,
			final ActionForm form, final HttpServletRequest request,
			final HttpServletResponse response) throws Exception {
		ActionMessages errors = new ActionMessages();
		ActionMessages messages = new ActionMessages();
		String id = request.getParameter("id");
		if (id == null) {
			errors.add("remove error", new ActionMessage("errors.UnKnowError"));
		}
		try {
			departmentService.deleteDepartment(id);
		} catch (BusinessException be) {
			errors.add("remove error", new ActionMessage("errors.UnKnowError"));
		}

		if (!errors.isEmpty()) {
			saveErrors(request, errors);
			saveToken(request);
			return (mapping.getInputForward());
		}

		messages.add(ActionMessages.GLOBAL_MESSAGE, new ActionMessage(
				"success.delete", "1"));

		saveMessages(request, messages);
		// Remove the obsolete form bean
		removeAttribute(mapping, request);

		return (mapping.findForward("success"));
	}

	/**
	 * 
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 * @throws Exception
	 */
	public ActionForward getDepartmentXMLTree(ActionMapping mapping,
			ActionForm form, HttpServletRequest request,
			HttpServletResponse response) throws Exception {
		if (log.isDebugEnabled()) {
			log.debug("Entering 'getXMLMenu' method");
		}

		String deptTree = departmentService.getDepartmentXmlTree();

		request.setAttribute("menuTree", deptTree);

		return mapping.findForward("success");
	}

	/**
	 * ���벿���б�
	 * 
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 * @throws Exception
	 */
	public ActionForward load4list(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		return mapping.findForward("list");
	}

	/**
	 * ȡ��
	 * 
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 * @throws Exception
	 */
	public ActionForward cancel(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {

		return mapping.findForward("success");
	}

	/**
	 * 
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 * @throws Exception
	 */
	public ActionForward loadDepartmentTree(ActionMapping mapping,
			ActionForm form, HttpServletRequest request,
			HttpServletResponse response) throws Exception {

		return mapping.findForward("selectTree");
	}

	/**
	 * hanjiwei modify it 20061009
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 * @throws Exception
	 */
	public ActionForward goDeptConf(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		String getNowDate = DateUtilExtend.getNowDate();
		String getDateBefore = DateUtilExtend.formatDate3(DateUtilExtend.addDate2(DateUtilExtend.formatDate2(getNowDate),30));
		Map filterMap = new HashMap();
		filterMap.put("startDate",DateUtilExtend.formatDate2(getNowDate));
		filterMap.put("endDate",DateUtilExtend.formatDate2(getDateBefore));
		List dptConfList = departConfService.search(filterMap);
		
		List checkType = sysService.getCheckTypeInfo();

		request.setAttribute("dpConfList", dptConfList);
		
		request.setAttribute("startDate",getNowDate);
		request.setAttribute("endDate",getDateBefore);
		request.setAttribute("checkType", checkType);
		return mapping.findForward("goDeptConf");
	}

	/*
	 * ���Ӳ�������ѡ����
	 */
	public ActionForward addConfSltDpt(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {

		request.getSession().removeAttribute("dptConfList");
		request.getSession().removeAttribute("new_department");
		request.getSession().removeAttribute("dptName");
		request.getSession().removeAttribute("dptConfList");
		return mapping.findForward("addConfSltDpt");
	}

	/*
	 * hanjiwei modify it 20061011
	 * ���в�������
	 */
	public ActionForward addConfStep1(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		//List departmentTree = departmentService.getDepartmentTree(0, null);
		//HttpSession session = request.getSession();
		//String  accountId = (String)session.getAttribute("account");
		
		List checkType = sysService.getCheckTypeInfo();

		request.setAttribute("checkType", checkType);

		List workConfList = workConfService.getAll();
		List workTimeConfList = workTimeConfService.getAll();
		request.getSession().setAttribute("workConfList", workConfList);
		request.getSession().setAttribute("workTimeConfList", workTimeConfList);
		request.setAttribute("display", "block");

		return mapping.findForward("addConfStep1");
	}
	
	/*
	 * ȡ�ò����Ű���ϸ��Ϣ
	 * hanjiwei add it 20061009
	 */
	public ActionForward getDeptConfDetail(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		String typeId = request.getParameter("pre_typeId").toString();
		String confDate = request.getParameter("pre_beginDate").toString();
		Map getConfAndTimeId = departConfService.getConfAndTimeId(typeId,confDate);
		String typeName = getConfAndTimeId.get("TYPE_NAME").toString();
		String workConfId = getConfAndTimeId.get("WORKINFO_ID").toString();
		String timeConfId = getConfAndTimeId.get("WORKTIME_ID").toString();
		String ifWork = getConfAndTimeId.get("IFWORK").toString();
		String workConfDetail = workConfService.getWorkConfInfo(workConfId);
		String workTimeDetail = workTimeConfService.getWorkTimeInfo(timeConfId);
		request.setAttribute("ifWORK",ifWork);
		request.setAttribute("workConfDetail",workConfDetail);
		request.setAttribute("workTimeDetail",workTimeDetail);
		request.setAttribute("typeName",typeName);
		request.setAttribute("confDate",confDate);
		return mapping.findForward("goDeptConfDetail");
	}
	/*
	 * �޸Ĳ����Ű���Ϣ
	 * hanjiwei add it 20061009
	 */
	public ActionForward modifyDeptConf(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		String deptId = request.getParameter("pre_deptId").toString();
		String confDate = request.getParameter("pre_beginDate");
		Map getConfAndTimeId = departConfService.getConfAndTimeId(deptId,confDate);
		String deptName = getConfAndTimeId.get("DEPT_NAME").toString();
		String workConfId = getConfAndTimeId.get("WORKINFO_ID").toString();
		String timeConfId = getConfAndTimeId.get("WORKTIME_ID").toString();
		String ifWork = getConfAndTimeId.get("IFWORK").toString();
		String workConfDetail = workConfService.getWorkConfInfo(workConfId);
		String workTimeDetail = workTimeConfService.getWorkTimeInfo(timeConfId);
		request.setAttribute("ifWORK",ifWork);
		request.setAttribute("workConfDetail",workConfDetail);
		request.setAttribute("workTimeDetail",workTimeDetail);
		request.setAttribute("workConfId",workConfId);
		request.setAttribute("timeConfId",timeConfId);
		request.setAttribute("deptName",deptName);
		request.setAttribute("confDate",confDate);
		return mapping.findForward("goDeptConfModify");
	}
	

	/*
	 * hanjiwei modify it 20061011
	 * �õ��ƶ���Ϣ
	 */
	public ActionForward getConfDetail(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		String workConfId = request.getParameter("new_clerkRule");// ���ӹ����ƶ�ϸ��
		if (StringUtils.isNotEmpty(workConfId)) {
			String workConfDetail = workConfService.getWorkConfInfo(workConfId);
			request.setAttribute("workconfDetail", workConfDetail);
		}

		String timeConfId = request.getParameter("new_cherkTimeRule");// ���ӹ���ʱ��ϸ��
		if (StringUtils.isNotEmpty(timeConfId)) {
			String workTimeDetail = workTimeConfService.getWorkTimeInfo(timeConfId);
			request.setAttribute("timeDetail", workTimeDetail);
		}

		List workConfList = workConfService.getAll();
		List workTimeConfList = workTimeConfService.getAll();
		//List departmentTree = departmentService.getDepartmentTree(0, null);
		//HttpSession session = request.getSession();
		//String  accountId = (String)session.getAttribute("account");
		
		List checkType = sysService.getCheckTypeInfo();
		String beginDate = request.getParameter("new_startDate");
		String endDate = request.getParameter("new_endDate");
		request.setAttribute("beginDate",beginDate);
		request.setAttribute("endDate",endDate);
		request.setAttribute("workConfList", workConfList);
		request.setAttribute("workTimeConfList", workTimeConfList);
		request.setAttribute("checkType", checkType);

		return mapping.findForward("addConfStep1");
	}

	/*
	 * hanjiwei modify it 20061011
	 * ���ӿ����ƶ�
	 */
	public ActionForward addConf(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		Map filterMap = WebUtils.getParametersStartingWith(request, "new_");
		String startDate = DateUtilExtend.formatDate2((String) filterMap.get("startDate"));
		String endDate = DateUtilExtend.formatDate2((String) filterMap.get("endDate"));
		String typeId = (String)filterMap.get("typeId");
		/*hanjiwei delete it 20080109
		String ifWork = "";
		
		if (null==filterMap.get("ifWork")){
			ifWork = "0";
		}else{
			ifWork = (String) filterMap.get("ifWork");
		}
		*/
		//�ж��Ƿ��Ѿ������ѷ�����Ŀ����ƶȣ�����������޸ģ���������
		String sixwork = (String)filterMap.get("sixWork");
		String sevenwork = (String)filterMap.get("sevenWork");
		long diffDate = DateUtilExtend.diffDate(startDate,endDate);
		for (long diffDay = 0;diffDay<=diffDate;diffDay++){
			String workDate = DateUtilExtend.addDate2(startDate,(int)diffDay);
			
			int week = DateUtilExtend.getWeekByDate2(workDate);
			if (week==1 && !"1".equals(sevenwork)){
				continue;
			}
			if (week==7 && !"1".equals(sixwork)){
				continue;
			}

			TCheckWorkconfinfoId id = new TCheckWorkconfinfoId();
			id.setBegindate(workDate);
			id.setCheckTypeid(typeId);
			TCheckWorkconfinfo deptConfInfo = new TCheckWorkconfinfo();
			deptConfInfo.setId(id);
			deptConfInfo.setIfwork("1");
			deptConfInfo.setWorkinfoId((String) filterMap.get("clerkRule"));
			deptConfInfo.setWorktimeId((String) filterMap.get("cherkTimeRule"));
			if (departConfService.ifExistDeptConf(typeId,workDate)){
				departConfService.update(deptConfInfo);
			}else{
				departConfService.save(deptConfInfo);
			}
		}

		String msg = "���óɹ�,�����Լ�������";
		String buttonInfo = "��������";
		String actionUrl = "window.location.href='department.do?method=addConfStep1';";// ������Ϣ
		request.setAttribute("btnInfo", buttonInfo);// ��ť��Ϣ
		request.setAttribute("actionUrl", actionUrl);
		request.setAttribute("msg", msg);// ��ʾ��Ϣ
		request.setAttribute("startDate", startDate);
		request.setAttribute("typeName",sysService.getCheckTypeName(typeId));
		request.setAttribute("endDate", endDate);
		request.setAttribute("display", "none");
		request.setAttribute("timeDetail", (String) filterMap.get("timeDetail"));
		request.setAttribute("workconfDetail", (String) filterMap.get("workconfDetail"));
		if ("1".equals(Integer.toString(ClerkConfig.isSeat))) {
			request.setAttribute("ifWork", "�ϰ�");
		} else {
			request.setAttribute("ifWork", "���ϰ�");
		}

		return mapping.findForward("finish");
	}

	/**
	 * hanjiwei modify it 20061009
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 * @throws Exception
	 */
	public ActionForward search(ActionMapping mapping, ActionForm form,
			HttpServletRequest request, HttpServletResponse response)
			throws Exception {
		Map filterMap = WebUtils.getParametersStartingWith(request, "search_");
		List dptConfList = departConfService.search(filterMap);
		//List departmentTree = departmentService.getDepartmentTree(0, null);
		//HttpSession session = request.getSession();
		//String  accountId = (String)session.getAttribute("account");
		
		List checkType = sysService.getCheckTypeInfo();

		request.setAttribute("checkType", checkType);
		request.setAttribute("dpConfList", dptConfList);
		request.setAttribute("startDate", (String) filterMap.get("startDate"));
		request.setAttribute("endDate", (String) filterMap.get("endDate"));
		return mapping.findForward("goDeptConf");
	}

	/**
	 * �����ƶ���Ϣɾ��
	 * @param mapping
	 * @param form
	 * @param request
	 * @param response
	 * @return
	 * @throws Exception
	 */
	public ActionForward delDepConf(ActionMapping mapping, ActionForm form,
	HttpServletRequest request, HttpServletResponse response)
	throws Exception {
		Map filterMap = WebUtils.getParametersStartingWith(request, "search_");
		List checkType = sysService.getCheckTypeInfo();

		request.setAttribute("checkType", checkType);
		request.setAttribute("startDate", (String) filterMap.get("startDate"));
		request.setAttribute("endDate", (String) filterMap.get("endDate"));
		
		String deptConf[] = request.getParameterValues("itemlist");
		if (deptConf.length>0){
			for (int i=0;i<deptConf.length;i++){
				String tempId = StringUtil.split(deptConf[i],"/")[0];
				String beginDate = StringUtil.split(deptConf[i],"/")[1];
				Map deptConfMap = departConfService.getConfAndTimeId(tempId,beginDate);
				TCheckWorkconfinfo checkWorkInfo = new TCheckWorkconfinfo();
				TCheckWorkconfinfoId checkWorkInfoId = new TCheckWorkconfinfoId();
				checkWorkInfoId.setCheckTypeid(deptConfMap.get("CHECK_TYPEID").toString());
				checkWorkInfoId.setBegindate(deptConfMap.get("BEGINDATE").toString());
				checkWorkInfo.setId(checkWorkInfoId);
				departConfService.delete(checkWorkInfo);
			}
		}
		List dptConfList = departConfService.search(filterMap);		
		request.setAttribute("dpConfList", dptConfList);
		request.setAttribute("msg","�Ű���Ϣɾ���ɹ���");
		
		return mapping.findForward("goDeptConf");
	}
}