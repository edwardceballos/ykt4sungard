package org.king.yangong.dormitorymanage.domain;



/**
 * AbstractYxGraduate generated by MyEclipse - Hibernate Tools
 */

public abstract class AbstractYxGraduate  implements java.io.Serializable {


    // Fields    

     private String id;
     private String graduateNo;
     private String graduateName;
     private String sex;
     private String enrollYear;
     private String enrollMonth;
     private String nation;
     private String nationality;
     private String birthday;
     private String curGrade;
     private String curCollegeDepartment;
     private String IFPAYDORMFEE;
     private String speciality;
     private String cultureMode;
     private String studentType;
     private String matriculateType;
     private String curDepartment;
     private String curCollege;
     private String adminCollege;
     private double lengthofschool;
     private String graduateType;
     private String distriction;
     private String requestAvailable;
     private String residentNeed;
     private String dormitoryId;
     private String updateTime;
     private String updateDate;
     private String memo;
     private String registerTime;


    // Constructors

    /** default constructor */
    public AbstractYxGraduate() {
    }

    
    /** full constructor */
    public AbstractYxGraduate(String graduateNo, String graduateName, String sex, String enrollYear, String enrollMonth, String nation, String nationality, String birthday, String curGrade, String curCollegeDepartment,String IFPAYDORMFEE, String speciality, String cultureMode, String studentType, String matriculateType, String curDepartment, String curCollege, String adminCollege, double lengthofschool, String graduateType, String distriction, String requestAvailable, String residentNeed, String dormitoryId, String updateTime, String updateDate,String memo,String registerTime) {
        this.graduateNo = graduateNo;
        this.graduateName = graduateName;
        this.sex = sex;
        this.enrollYear = enrollYear;
        this.enrollMonth = enrollMonth;
        this.nation = nation;
        this.nationality = nationality;
        this.birthday = birthday;
        this.curGrade = curGrade;
        this.curCollegeDepartment = curCollegeDepartment;
        this.speciality = speciality;
        this.cultureMode = cultureMode;
        this.studentType = studentType;
        this.matriculateType = matriculateType;
        this.IFPAYDORMFEE = IFPAYDORMFEE;
        this.curDepartment = curDepartment;
        this.curCollege = curCollege;
        this.adminCollege = adminCollege;
        this.lengthofschool = lengthofschool;
        this.graduateType = graduateType;
        this.distriction = distriction;
        this.requestAvailable = requestAvailable;
        this.residentNeed = residentNeed;
        this.dormitoryId = dormitoryId;
        this.updateTime = updateTime;
        this.updateDate = updateDate;
        this.memo = memo;
        this.registerTime = registerTime;
    }

   
    // Property accessors

    public String getId() {
        return this.id;
    }
    
    public void setId(String id) {
        this.id = id;
    }

    public String getGraduateNo() {
        return this.graduateNo;
    }
    
    public void setGraduateNo(String graduateNo) {
        this.graduateNo = graduateNo;
    }

    public String getGraduateName() {
        return this.graduateName;
    }
    
    public void setGraduateName(String graduateName) {
        this.graduateName = graduateName;
    }

    public String getSex() {
        return this.sex;
    }
    
    public void setSex(String sex) {
        this.sex = sex;
    }

    public String getEnrollYear() {
        return this.enrollYear;
    }
    
    public void setEnrollYear(String enrollYear) {
        this.enrollYear = enrollYear;
    }

    public String getEnrollMonth() {
        return this.enrollMonth;
    }
    
    public void setEnrollMonth(String enrollMonth) {
        this.enrollMonth = enrollMonth;
    }

    public String getNation() {
        return this.nation;
    }
    
    public void setNation(String nation) {
        this.nation = nation;
    }

    public String getNationality() {
        return this.nationality;
    }
    
    public void setNationality(String nationality) {
        this.nationality = nationality;
    }

    public String getBirthday() {
        return this.birthday;
    }
    
    public void setBirthday(String birthday) {
        this.birthday = birthday;
    }

    public String getCurGrade() {
        return this.curGrade;
    }
    
    public void setCurGrade(String curGrade) {
        this.curGrade = curGrade;
    }

    public String getCurCollegeDepartment() {
        return this.curCollegeDepartment;
    }
    
    public void setCurCollegeDepartment(String curCollegeDepartment) {
        this.curCollegeDepartment = curCollegeDepartment;
    }

    public String getSpeciality() {
        return this.speciality;
    }
    
    public void setSpeciality(String speciality) {
        this.speciality = speciality;
    }

    public String getCultureMode() {
        return this.cultureMode;
    }
    
    public void setCultureMode(String cultureMode) {
        this.cultureMode = cultureMode;
    }

    public String getStudentType() {
        return this.studentType;
    }
    
    public void setStudentType(String studentType) {
        this.studentType = studentType;
    }

    public String getMatriculateType() {
        return this.matriculateType;
    }
    
    public void setMatriculateType(String matriculateType) {
        this.matriculateType = matriculateType;
    }

    public String getCurDepartment() {
        return this.curDepartment;
    }
    
    public void setCurDepartment(String curDepartment) {
        this.curDepartment = curDepartment;
    }

    public String getCurCollege() {
        return this.curCollege;
    }
    
    public void setCurCollege(String curCollege) {
        this.curCollege = curCollege;
    }

    public String getAdminCollege() {
        return this.adminCollege;
    }
    
    public void setAdminCollege(String adminCollege) {
        this.adminCollege = adminCollege;
    }

    public double getLengthofschool() {
        return this.lengthofschool;
    }
    
    public void setLengthofschool(double lengthofschool) {
        this.lengthofschool = lengthofschool;
    }

    public String getGraduateType() {
        return this.graduateType;
    }
    
    public void setGraduateType(String graduateType) {
    	if(graduateType!=null&&!"".equals(graduateType)){
    		this.graduateType = graduateType;
    	}else{
    		this.graduateType="";
    		
    	}
    }

    public String getDistriction() {
        return this.distriction;
    }
    
    public void setDistriction(String distriction) {
        this.distriction = distriction;
    }

    public String getRequestAvailable() {
        return this.requestAvailable;
    }
    
    public void setRequestAvailable(String requestAvailable) {
        this.requestAvailable = requestAvailable;
    }

    public String getResidentNeed() {
        return this.residentNeed;
    }
    
    public void setResidentNeed(String residentNeed) {
        this.residentNeed = residentNeed;
    }

    public String getDormitoryId() {
        return this.dormitoryId;
    }
    
    public void setDormitoryId(String dormitoryId) {
        this.dormitoryId = dormitoryId;
    }

    public String getUpdateTime() {
        return this.updateTime;
    }
    
    public void setUpdateTime(String updateTime) {
        this.updateTime = updateTime;
    }

    public String getUpdateDate() {
        return this.updateDate;
    }
    
    public void setUpdateDate(String updateDate) {
        this.updateDate = updateDate;
    }


	public String getIFPAYDORMFEE() {
		return IFPAYDORMFEE;
	}


	public void setIFPAYDORMFEE(String ifpaydormfee) {
		IFPAYDORMFEE = ifpaydormfee;
	}


	public String getMemo() {
		return memo;
	}


	public void setMemo(String memo) {
		this.memo = memo;
	}


	public String getRegisterTime() {
		return registerTime;
	}


	public void setRegisterTime(String registerTime) {
		this.registerTime = registerTime;
	}
   








}