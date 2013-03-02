package org.king.security.domain;

import java.util.Set;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.apache.commons.lang.builder.ToStringBuilder;
import org.apache.commons.lang.builder.ToStringStyle;



/**
 * AbstractRole generated by MyEclipse - Hibernate Tools
 */

public abstract class AbstractRole extends org.king.framework.domain.BaseObject implements java.io.Serializable {


    // Fields    

     private String id;
     private String name;
     private String type;
     private String remark;

     private Set accounts;
     private Set resources;
     private Set menus;
     
    // Constructors

    /** default constructor */
    public AbstractRole() {
    }

	/** minimal constructor */
    public AbstractRole(String name) {
        this.name = name;
    }
    
    /** full constructor */
    public AbstractRole(String name, String type, String remark) {
        this.name = name;
        this.type = type;
        this.remark = remark;
    }

   
    // Property accessors

    public String getId() {
        return this.id;
    }
    
    public void setId(String id) {
        this.id = id;
    }

    public String getName() {
        return this.name;
    }
    
    public void setName(String name) {
        this.name = name;
    }

    public String getType() {
        return this.type;
    }
    
    public void setType(String type) {
        this.type = type;
    }

    public String getRemark() {
        return this.remark;
    }
    
    public void setRemark(String remark) {
        this.remark = remark;
    }

    
	public Set getResources() {
		return resources;
	}

	public void setResources(Set resources) {
		this.resources = resources;
	}

	
	public Set getMenus() {
		return menus;
	}

	public void setMenus(Set menus) {
		this.menus = menus;
	}

	
	public Set getAccounts() {
		return accounts;
	}

	public void setAccounts(Set accounts) {
		this.accounts = accounts;
	}

	/**
	 * @see java.lang.Object#equals(Object)
	 */
	public boolean equals(Object object) {
		if (!(object instanceof AbstractRole)) {
			return false;
		}
		AbstractRole rhs = (AbstractRole) object;
		return new EqualsBuilder().append(this.type, rhs.type).append(
				this.remark, rhs.remark).append(this.name, rhs.name).append(
				this.id, rhs.id).isEquals();
	}

	/**
	 * @see java.lang.Object#hashCode()
	 */
	public int hashCode() {
		return new HashCodeBuilder(-855329449, -733559267).append(this.type)
				.append(this.remark).append(this.name).append(this.id)
				.toHashCode();
	}

	/**
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		return new ToStringBuilder(this, ToStringStyle.MULTI_LINE_STYLE)
				.append("name", this.name).append("id", this.id).append("type",
						this.type).append("remark", this.remark).toString();
	}
   








}