package com.kingstargroup.mobileMessage.hibernate;

import java.util.Date;

/**
 * AbstractRCmMessages generated by MyEclipse - Hibernate Tools
 */

public abstract class AbstractRCmMessages implements java.io.Serializable {

	private Integer id;

	private String RDate;

	private String RTime;

	private Integer RObjectType;

	private String RCardId;

	private String RObject;

	private String RContent;

	private Integer RPro;

	

	public Integer getId() {
		return id;
	}

	public void setId(Integer id) {
		this.id = id;
	}

	public String getRCardId() {
		return RCardId;
	}

	public void setRCardId(String cardId) {
		RCardId = cardId;
	}

	public String getRContent() {
		return RContent;
	}

	public void setRContent(String content) {
		RContent = content;
	}

	public String getRDate() {
		return RDate;
	}

	public void setRDate(String date) {
		RDate = date;
	}

	public String getRObject() {
		return RObject;
	}

	public void setRObject(String object) {
		RObject = object;
	}

	public Integer getRObjectType() {
		return RObjectType;
	}

	public void setRObjectType(Integer objectType) {
		RObjectType = objectType;
	}

	public Integer getRPro() {
		return RPro;
	}

	public void setRPro(Integer pro) {
		RPro = pro;
	}

	public String getRTime() {
		return RTime;
	}

	public void setRTime(String time) {
		RTime = time;
	}
	public AbstractRCmMessages() {
	
	}

	public AbstractRCmMessages(Integer id) {
		super();
		// TODO Auto-generated constructor stub
		this.id = id;
	}

}