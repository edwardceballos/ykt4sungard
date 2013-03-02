package org.king.check.domain;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.apache.commons.lang.builder.ToStringBuilder;



/**
 * AbstractAttserial generated by MyEclipse - Hibernate Tools
 */

public abstract class AbstractAttserial extends org.king.framework.domain.BaseObject implements java.io.Serializable {


    // Fields    

     private Integer serialId;
     private Integer cardId;
     private String attDate;
     private String attTime;


    // Constructors

    /** default constructor */
    public AbstractAttserial() {
    }

    
    /** full constructor */
    public AbstractAttserial(Integer cardId, String attDate, String attTime) {
        this.cardId = cardId;
        this.attDate = attDate;
        this.attTime = attTime;
    }

   
    // Property accessors

    public Integer getSerialId() {
        return this.serialId;
    }
    
    public void setSerialId(Integer serialId) {
        this.serialId = serialId;
    }

    public Integer getCardId() {
        return this.cardId;
    }
    
    public void setCardId(Integer cardId) {
        this.cardId = cardId;
    }

    public String getAttDate() {
        return this.attDate;
    }
    
    public void setAttDate(String attDate) {
        this.attDate = attDate;
    }

    public String getAttTime() {
        return this.attTime;
    }
    
    public void setAttTime(String attTime) {
        this.attTime = attTime;
    }


	/**
	 * @see java.lang.Object#equals(Object)
	 */
	public boolean equals(Object object) {
		if (!(object instanceof AbstractAttserial)) {
			return false;
		}
		AbstractAttserial rhs = (AbstractAttserial) object;
		return new EqualsBuilder().append(
				this.cardId, rhs.cardId).append(this.attTime, rhs.attTime)
				.append(this.serialId, rhs.serialId).append(this.attDate,
						rhs.attDate).isEquals();
	}


	/**
	 * @see java.lang.Object#hashCode()
	 */
	public int hashCode() {
		return new HashCodeBuilder(421658657, 337135507).append(this.cardId)
				.append(this.attTime).append(this.serialId)
				.append(this.attDate).toHashCode();
	}


	/**
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		return new ToStringBuilder(this).append("attTime", this.attTime)
				.append("serialId", this.serialId).append("attDate",
						this.attDate).append("cardId", this.cardId).toString();
	}
   








}